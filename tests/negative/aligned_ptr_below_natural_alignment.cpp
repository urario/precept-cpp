// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/aligned_ptr.hpp>

#include <cstddef>

struct alignas(64) over_aligned {};

precept::aligned_ptr<over_aligned, 32> invalid;
