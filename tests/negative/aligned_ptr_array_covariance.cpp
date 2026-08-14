// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/aligned_ptr.hpp>

struct base {};
struct derived : base {};

void reject(precept::aligned_ptr<derived, 64> source) {
  precept::aligned_ptr<base, 64> invalid = source;
}
