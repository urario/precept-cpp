// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/index_below.hpp>

void reject(precept::index_below<8> source) { precept::index_below<4> invalid = source; }
