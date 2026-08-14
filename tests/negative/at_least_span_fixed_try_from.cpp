// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/at_least_span.hpp>

#include <span>

int values[4];
auto invalid = precept::at_least_span<int, 4>::try_from(std::span<int, 4>{values});
