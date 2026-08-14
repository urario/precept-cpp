// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/at_least_span.hpp>

#include <span>

std::span<int> source;
precept::at_least_span<int, 1> invalid = source;
