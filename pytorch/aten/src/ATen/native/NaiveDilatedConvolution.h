#pragma once
#include <ATen/ATen.h>
#include <ATen/native/CPUBlas.h>
#include <ATen/native/DilatedConvolutionUtils.h>
#include <ATen/native/im2col.h>
#include <ATen/native/vol2col.h>
#include <ATen/Utils.h>
#include <c10/util/accumulate.h>
#include <c10/util/irange.h>
#include <tuple>

namespace at {
namespace native {
namespace {

// hyper-volume to column, CPU
template <typename Dtype, int64_t dim>
void hvol2col(
    const Dtype* data_hvol,
    const int channels,
    const IntArrayRef input_size,
    const IntArrayRef output_size,
    const IntArrayRef kernel_size,
    const IntArrayRef stride_size,
    const IntArrayRef pad_size,
    const IntArrayRef dilation_size,
    Dtype* data_col) {
  if (dim == 3) {
    vol2col<Dtype>(
        data_hvol,
        channels,
        input_size[0],
        input_size[1],
        input_size[2],
        output_size[0],
        output_size[1],
        output_size[2],
        kernel_size[0],
        kernel_size[1],
        kernel_size[2],
        pad_size[0],
        pad_size[1],
        pad_size[2],
        stride_size[0],
        stride_size[1],
        stride_size[2],
        dilation_size[0],
        dilation_size[1],
        dilation_size[2],
        data_col);
  }
  if (dim == 2) {
    im2col<Dtype>(
        data_hvol,
        channels,
        input_size[0],
        input_size[1],
        output_size[0],
        output_size[1],
        kernel_size[0],
        kernel_size[1],
        pad_size[0],
        pad_size[1],
        stride_size[0],
        stride_size[1],
        dilation_size[0],
        dilation_size[1],
        data_col);
  }
}

// column to hyper-volume, CPU
template <typename Dtype, int64_t dim>
void col2hvol(
    const Dtype* data_col,
    const int channels,
    const IntArrayRef input_size,
    const IntArrayRef output_size,
    const IntArrayRef kernel_size,
    const IntArrayRef stride_size,
    const IntArrayRef pad_size,
    const IntArrayRef dilation_size,
    Dtype* data_hvol) {
  if (dim == 3) {
    col2vol<Dtype>(
        data_col,
        channels,
        input_size[0],
        input_size[1],
        input_size[2],
        output_size[0],
        output_size[1],
        output_size[2],
        kernel_size[0],
        kernel_size[1],
        kernel_size[2],
        pad_size[0],
        pad_size[1],
        pad_size[2],
        stride_size[0],
        stride_size[1],
        stride_size[2],
        dilation_size[0],
        dilation_size[1],
        dilation_size[2],
        data_hvol);
  }
  if (dim == 2) {
    col2im<Dtype>(
        data_col,
        channels,
        input_size[0],
        input_size[1],
        output_size[0],
        output_size[1],
        kernel_size[0],
        kernel_size[1],
        pad_size[0],
        pad_size[1],
        stride_size[0],
        stride_size[1],
        dilation_size[0],
        dilation_size[1],
        data_hvol);
  }
}

/*
   check tensor data locations
*/
void slow_conv_dilated_location_check(
    const Tensor& input,
    const Tensor& weight,
    const Tensor& bias,
    const Tensor& grad_output) {
  // checking data locations of user-provided tensor arguments
  checkBackend("slow_conv_dilated_location_check", {input, weight}, Backend::CPU);
  if (bias.defined()) {
    checkBackend("slow_conv_dilated_location_check", {bias}, Backend::CPU);
  }
  if (grad_output.defined()) {
    checkBackend("slow_conv_dilated_location_check", {grad_output}, Backend::CPU);
  }
  // we are not checking the data locations of other tensor
  // arguments such as output, grad_input, etc because of these are
  // allocated based on input options and hence these tensors always
  // have the same data location as of input tensor.
}

/*
  slow_conv_dilated_all_cpu_template

  Main worker. Computes tensors output, grad_input, grad_weight,
  and/or grad_bias if defined, respectively.
 */

template <int64_t dim>
void slow_conv_dilated_all_cpu_template(
    Tensor& output,
    const Tensor& input,
    const Tensor& weight,
    const Tensor& bias,
    const Tensor& grad_output,
    Tensor& grad_input,
    Tensor& grad_weight,
    Tensor& grad_bias,
    IntArrayRef kernel_size,
    IntArrayRef stride_size,
    IntArrayRef pad_size,
    IntArrayRef dilation_size) {
  slow_conv_dilated_location_check(input, weight, bias, grad_output);
  auto options = input.options();
  // The rear part of input tensor sizes:
  auto input_size = input.sizes().slice(2);
  // The rear part of output tensor sizes:
  auto output_size = internal::get_output_size<dim>(
      input, kernel_size, stride_size, pad_size, dilation_size);
  int64_t batchSize = input.size(0);
  int64_t nInputPlane = weight.size(1);
  int64_t nOutputPlane = weight.size(0);
  // Temporary buffer:
  Tensor columns = at::empty({0}, options);
  if (output.defined() || grad_weight.defined() || grad_input.defined()) {
    const int64_t m = c10::multiply_integers(kernel_size);
    const int64_t n = c10::multiply_integers(output_size);
    columns.resize_({nInputPlane * m, n});
  }
  // Initialize
  if (grad_weight.defined()) {
    grad_weight.zero_();
  }
  if (grad_bias.defined()) {
    grad_bias.zero_();
  }
  if (output.defined() && !bias.defined()) {
    output.zero_();
  }
  // Helpers
  Tensor grad_output_n;
  std::vector<int64_t> dims(dim);
  std::iota(dims.begin(), dims.end(), 1);

    AT_DISPATCH_FLOATING_TYPES_AND(at::ScalarType::Long, input.scalar_type(), "slow_conv_dilated<>", [&] {
    // For each elt in batch, do:
    for (const auto elt : c10::irange(batchSize)) {
      // Matrix multiply per output:
      Tensor input_n = input.select(0, elt);

      // Output
      if (output.defined()) {
        Tensor output_n = output.select(0, elt);
        if (bias.defined()) {
          /*
            Compute:

              output_n = bias * ones^T

            where

              bias is viewed as bias.view(nOutputPlane, 1)

              ones is viewed as ones.view(outputHeight * outputWidth, 1)

              output_n is viewed as output_n.view(nOutputPlane, outputHeight
          * outputWidth)

          gemm assumes column-major matrices:

            output_n^T = ones * bias^T
            C = alpha * op(A) * op(B)
            op(A) = 't', op(B) = 'n', alpha=1, beta=0
          */
          // The following for-loop is equivalent to the above
          // gemm setup but avoids allocation of ones tensor:
          for (const auto n : c10::irange(nOutputPlane)) {
            output_n.select(0, n).fill_(bias[n]);
          }
        }
        // Extract columns:
        hvol2col<scalar_t, dim>(
            input_n.data_ptr<scalar_t>(),
            nInputPlane,
            input_size,
            output_size,
            kernel_size,
            stride_size,
            pad_size,
            dilation_size,
            columns.data_ptr<scalar_t>());
        /*
          Compute:

            output_n = weight * columns + output_n

          where

            weight is viewed as weight.view(nOutputPlane, nInputPlane * kD *
          kH * kW)

            columns size is (nInputPlane * kH * kW) x (outputHeight *
          outputWidth)

            output_n is viewed as output_n.view(nOutputPlane, outputHeight *
          outputWidth)

          gemm assumes column-major matrices:

            output_n^T = columns^T * weight^T + output_n^T
            C = alpha * op(A) * op(B) + beta * C
            op(A) = 'n', op(B) = 'n', alpha=1, beta=1
        */
        cpublas::gemm(
            /*transa=*/TransposeType::NoTranspose,
            /*transb=*/TransposeType::NoTranspose,
            /*     m=*/columns.size(1),
            /*     n=*/nOutputPlane,
            /*     k=*/columns.size(0),
            /* alpha=*/1,
            /*     A=*/columns.data_ptr<scalar_t>(),
            /*   lda=*/columns.size(1),
            /*     B=*/weight.data_ptr<scalar_t>(),
            /*   ldb=*/columns.size(0),
            /*  beta=*/1,
            /*     C=*/output_n.data_ptr<scalar_t>(),
            /*   ldc=*/columns.size(1));

      } else {
        // All gradients
        grad_output_n = grad_output.select(0, elt);
      }

      // Gradient of input:
      if (grad_input.defined()) {
        /*
          Compute:

            columns = weight^T * grad_output_n

          where

            weight is viewed as weight.view(nOutputPlane, nInputPlane * kH *
          kW)

            grad_output_n is viewed as grad_output_n.view(nOutputPlane,
          outputHeight * outputWidth)

            columns size is (nInputPlane * kH * kW) x (outputHeight *
          outputWidth)

          gemm assumes column-major matrices:

            columns^T = grad_output_n^T * weight
            C = alpha * op(A) * op(B) + beta * C
            op(A) = 'n', op(B) = 't', alpha=1, beta=0
         */
        cpublas::gemm(
            /*transa=*/TransposeType::NoTranspose,
            /*transb=*/TransposeType::Transpose,
            /*     m=*/columns.size(1),
            /*     n=*/columns.size(0),
            /*     k=*/nOutputPlane,
            /* alpha=*/1,
            /*     A=*/grad_output_n.data_ptr<scalar_t>(),
            /*   lda=*/columns.size(1),
            /*     B=*/weight.data_ptr<scalar_t>(),
            /*   ldb=*/columns.size(0),
            /*  beta=*/0,
            /*     C=*/columns.data_ptr<scalar_t>(),
            /*   ldc=*/columns.size(1));
        // Unpack columns back into input:
        Tensor grad_input_n = grad_input.select(0, elt);

        col2hvol<scalar_t, dim>(
            columns.data_ptr<scalar_t>(),
            nInputPlane,
            input_size,
            output_size,
            kernel_size,
            stride_size,
            pad_size,
            dilation_size,
            grad_input_n.data_ptr<scalar_t>());
      }

      // Gradient of weight:
      if (grad_weight.defined()) {
        // Extract columns:
        hvol2col<scalar_t, dim>(
            input_n.data_ptr<scalar_t>(),
            nInputPlane,
            input_size,
            output_size,
            kernel_size,
            stride_size,
            pad_size,
            dilation_size,
            columns.data_ptr<scalar_t>());
        scalar_t scale = 1; // TODO: expose as argument?
        /*
          Compute:

            grad_weight = scale * grad_output_n * columns^T + grad_weight

          where

            grad_output_n is viewed as grad_output_n.view(nOutputPlane,
          outputHeight * outputWidth)

            columns size is (nInputPlane * kD * kH * kW) x (outputHeight *
          outputWidth)

            grad_weight is viewed as grad_weight.view(nOutputPlane,
          nInputPlane * kH * kW)

          gemm assumes column-major matrices:

            grad_weight^T = scale * columns * grad_output_n^T +
          grad_weight^T C = alpha * op(A) * op(B) + beta * C op(A) = 't',
          op(B) = 'n', alpha=scale, beta=1
        */
        cpublas::gemm(
            /*transa=*/TransposeType::Transpose,
            /*transb=*/TransposeType::NoTranspose,
            /*     m=*/columns.size(0),
            /*     n=*/nOutputPlane,
            /*     k=*/columns.size(1),
            /* alpha=*/scale,
            /*     A=*/columns.data_ptr<scalar_t>(),
            /*   lda=*/columns.size(1),
            /*     B=*/grad_output_n.data_ptr<scalar_t>(),
            /*   ldb=*/columns.size(1),
            /*  beta=*/1,
            /*     C=*/grad_weight.data_ptr<scalar_t>(),
            /*   ldc=*/columns.size(0));
      }

      // Gradient of bias:
      if (grad_bias.defined()) {
        /*
          Compute:
            grad_bias = scale * grad_output_n * ones + grad_bias

          where

            grad_bias is viewed as grad_bias.view(nOutputPlane, 1)

            ones is viewed as ones.view(outputHeight * outputWidth, 1)

            grad_output_n is viewed as grad_output_n.view(nOutputPlane,
          outputHeight * outputWidth)

          gemm assumes column-major matrices:

            grad_bias^T = scale * grad_output_n * ones + grad_bias^T
            y = alpha * op(A) * x + beta * y
            op(A) = 't', alpha=scale, beta=1
         */
        // The following expression is equivalent to the above
        // gemm setup but avoids allocation of ones tensor:
        grad_bias += grad_output_n.sum(dims);
        /*
          TODO: when scale != 1 is introduced then use:
            grad_bias += scale * grad_output_n.sum(dims);
         */
      }
    }
  });

} // slow_conv_dilated_all_cpu_template

} // namespace
}} // namespace at::native
