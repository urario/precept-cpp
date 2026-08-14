// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/aligned_ptr.hpp>

void reject(precept::aligned_ptr<int, 32> source) {
  precept::aligned_ptr<int, 64> invalid = source;
}
