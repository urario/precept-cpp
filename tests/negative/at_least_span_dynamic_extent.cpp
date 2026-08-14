// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/at_least_span.hpp>

#include <span>

using invalid = precept::at_least_span<int, std::dynamic_extent>;
invalid value;
