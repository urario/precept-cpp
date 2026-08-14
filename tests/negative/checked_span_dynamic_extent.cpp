// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/checked_span.hpp>

#include <span>

std::span<int> source;
auto invalid = precept::checked_span<std::dynamic_extent>(source);
