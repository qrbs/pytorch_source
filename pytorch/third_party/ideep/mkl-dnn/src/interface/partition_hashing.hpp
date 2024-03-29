/*******************************************************************************
* Copyright 2021 Intel Corporation
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

#ifndef INTERFACE_PARTITION_HASHING_HPP
#define INTERFACE_PARTITION_HASHING_HPP

#include <algorithm>
#include <memory>
#include <thread>
#include <typeindex>
#include <vector>
#include <type_traits>

#include "oneapi/dnnl/dnnl_graph.h"

#include "interface/c_types_map.hpp"
#include "interface/id.hpp"
#include "interface/op.hpp"
#include "utils/utils.hpp"

namespace dnnl {
namespace graph {
namespace impl {
namespace partition_hashing {

namespace {
inline std::vector<op_t *> get_raw_ptrs(
        const std::vector<std::shared_ptr<op_t>> &ops) {
    std::vector<op_t *> ret(ops.size(), nullptr);
    std::transform(ops.begin(), ops.end(), ret.begin(),
            [](const std::shared_ptr<op_t> &op_ptr) { return op_ptr.get(); });
    return ret;
}
} // namespace

struct key_t {
    key_t(size_t partition_id, engine_kind_t engine_kind,
            const std::vector<std::shared_ptr<op_t>> &ops,
            const std::vector<const logical_tensor_t *> &ins,
            const std::vector<const logical_tensor_t *> &outs);
    key_t(const partition_t *partition,
            const std::vector<const logical_tensor_t *> &ins,
            const std::vector<const logical_tensor_t *> &outs);

    bool operator==(const key_t &other) const;
    const std::thread::id &thread_id() const { return thread_id_; }

    mutable size_t partition_id_;
    mutable std::vector<op_t *> ops_;
    mutable std::vector<logical_tensor_t> ins_;
    mutable std::vector<logical_tensor_t> outs_;
    engine_kind_t engine_kind_;

private:
    // Thread ID is not used as part of the key, it's only used to get
    // information about what thread inserted the key and the corresponding
    // primitive to handle some multithread scenatios.
    std::thread::id thread_id_;
};

size_t get_logical_tensor_hash(const logical_tensor_t &lt);
size_t get_op_hash(const op_t &op);

template <typename T>
size_t get_array_hash(size_t seed, const T *v, size_t size) {
    for (size_t i = 0; i < size; i++) {
        seed = utils::hash_combine(seed, v[i]);
    }
    return seed;
}

template <>
inline size_t get_array_hash<logical_tensor_t>(
        size_t seed, const logical_tensor_t *v, size_t size) {
    for (size_t i = 0; i < size; i++) {
        seed = utils::hash_combine(seed, get_logical_tensor_hash(v[i]));
    }
    return seed;
}

template <>
inline size_t get_array_hash<op_t>(size_t seed, const op_t *v, size_t size) {
    for (size_t i = 0; i < size; i++) {
        seed = utils::hash_combine(seed, get_op_hash(v[i]));
    }
    return seed;
}

template <>
inline size_t get_array_hash<float>(size_t seed, const float *v, size_t size) {
    for (size_t i = 0; i < size; i++) {
        seed = utils::hash_combine(seed, utils::float2int(v[i]));
    }
    return seed;
}

} // namespace partition_hashing
} // namespace impl
} // namespace graph
} // namespace dnnl

// inject a specialization of std::hash for key_t in std namespace
namespace std {
template <>
struct hash<dnnl::graph::impl::partition_hashing::key_t> {
    using argument_type = dnnl::graph::impl::partition_hashing::key_t;
    using result_type = std::size_t;
    result_type operator()(const argument_type &key) const {
        using namespace dnnl::graph::impl;
        using namespace dnnl::graph::impl::partition_hashing;
        using namespace dnnl::graph::impl::utils;
        size_t seed = 0;
        // Compute hash for partition_id_, engine_kind_
        seed = hash_combine(seed, key.partition_id_);
        seed = hash_combine(seed, static_cast<size_t>(key.engine_kind_));

        // Combine hash for op_kinds & attributes with the computed hash
        seed = get_array_hash(seed, key.ops_.data(), key.ops_.size());

        // Combine hash for input and output ports with the computed hash
        seed = get_array_hash(seed, key.ins_.data(), key.ins_.size());
        seed = get_array_hash(seed, key.outs_.data(), key.outs_.size());

        return seed;
    }
};

} // namespace std

#endif
