// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/block_span.hpp>

#include <span>

int values[4];
auto invalid = precept::block_span<int, 4>::try_from(std::span<int, 4>{values});
