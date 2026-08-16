// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/at_least_span.hpp>

#include <span>

using span = precept::at_least_span<int, 16>;
int values[16];
span view = std::span<int, 16>{values};
auto invalid = view.subspan<17>();
