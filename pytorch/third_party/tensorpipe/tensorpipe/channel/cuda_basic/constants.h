/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>

namespace tensorpipe {
namespace channel {
namespace cuda_basic {

// FIXME Avoid this anonymous namespace and use inline variables in C++-17.
namespace {

// Define all three (redundant) values to make them explicit and avoid
// misunderstandings due to miscalculations.
static constexpr size_t kStagingAreaSize = 16 * 1024 * 1024;
static constexpr size_t kSlotSize = 1024 * 1024;
static constexpr size_t kNumSlots = 16;

static_assert(kStagingAreaSize == kSlotSize * kNumSlots, "");

} // namespace

} // namespace cuda_basic
} // namespace channel
} // namespace tensorpipe
