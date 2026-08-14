// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/at_least_span.hpp>

#include <span>

int values[3];
precept::at_least_span<int, 4> invalid = std::span<int, 3>{values};
