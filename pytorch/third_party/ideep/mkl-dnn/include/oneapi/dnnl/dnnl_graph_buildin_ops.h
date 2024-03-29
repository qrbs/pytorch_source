/*******************************************************************************
* Copyright 2020-2021 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/// @file
/// Op definitions

/// @addtogroup dnnl_graph_api
/// @{

/// @addtogroup dnnl_graph_api_utils
/// @{

#ifndef ONEAPI_DNNL_DNNL_GRAPH_BUILDIN_OPS_H
#define ONEAPI_DNNL_DNNL_GRAPH_BUILDIN_OPS_H

#define DNNL_GRAPH_FORALL_BUILDIN_OPS(_) \
    _(Abs) \
    _(Add) \
    _(AvgPool) \
    _(AvgPoolBackprop) \
    _(BatchNormInference) \
    _(BatchNormForwardTraining) \
    _(BatchNormTrainingBackprop) \
    _(BiasAddBackprop) \
    _(Clamp) \
    _(ClampBackprop) \
    _(Concat) \
    _(Convolution) \
    _(ConvolutionBackpropData) \
    _(ConvolutionBackpropFilters) \
    _(Divide) \
    _(Elu) \
    _(EluBackprop) \
    _(Erf) \
    _(Exp) \
    _(GELU) \
    _(GELUBackprop) \
    _(HardTanh) \
    _(HardTanhBackprop) \
    _(LayerNorm) \
    _(LayerNormBackprop) \
    _(Log) \
    _(LogSoftmax) \
    _(LogSoftmaxBackprop) \
    _(MatMul) \
    _(Maximum) \
    _(MaxPool) \
    _(MaxPoolBackprop) \
    _(Minimum) \
    _(Multiply) \
    _(Pow) \
    _(PowBackprop) \
    _(ReduceSum) \
    _(ReLU) \
    _(ReLUBackprop) \
    _(Reshape) \
    _(Round) \
    _(Sigmoid) \
    _(SigmoidBackprop) \
    _(SoftMax) \
    _(SoftMaxBackprop) \
    _(SoftPlus) \
    _(SoftPlusBackprop) \
    _(Sqrt) \
    _(SqrtBackprop) \
    _(Square) \
    _(Tanh) \
    _(TanhBackprop) \
    _(Wildcard) \
    _(BiasAdd) \
    _(Interpolate) \
    _(Transpose) \
    _(Index) \
    _(InterpolateBackprop) \
    _(PowBackpropExponent) \
    _(End) \
    _(Quantize) \
    _(Dequantize) \
    _(Reorder)

#endif

/// @} dnnl_graph_api_utils

/// @} dnnl_graph_api
