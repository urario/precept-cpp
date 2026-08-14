// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/block_span.hpp>

#include <span>

int values[6];
precept::block_span<int, 4> invalid = std::span<int, 6>{values};
