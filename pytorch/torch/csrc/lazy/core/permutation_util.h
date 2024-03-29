#pragma once

#include <c10/util/ArrayRef.h>
#include <c10/util/Exception.h>

#include <vector>

namespace torch {
namespace lazy {

TORCH_API std::vector<int64_t> InversePermutation(
    c10::ArrayRef<int64_t> input_permutation);

TORCH_API bool IsPermutation(c10::ArrayRef<int64_t> permutation);

// Gathers the input using the order specified by the permutation. For each i,
// output[i] = dimensions[permutation[i]]. The given permutation must be the
// same size as the input.
template <typename Container>
std::vector<typename Container::value_type> PermuteDimensions(
    c10::ArrayRef<int64_t> permutation,
    const Container& dimensions) {
  using T = typename Container::value_type;
  TORCH_CHECK(
      dimensions.size() == permutation.size() && IsPermutation(permutation),
      "Invalid permutation specified");
  std::vector<T> output(dimensions.size());
  for (size_t i = 0; i < permutation.size(); ++i) {
    output[i] = dimensions[permutation[i]];
  }
  return output;
}

} // namespace lazy
} // namespace torch
