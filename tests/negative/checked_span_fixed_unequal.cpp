// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/checked_span.hpp>

#include <span>

int values[3];
auto invalid = precept::checked_span<4>(std::span<int, 3>{values});
