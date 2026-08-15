// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/nonzero.hpp>

// Subtraction is not closed over the guarantee: the difference of two non-zero values can be zero.
// The type therefore offers no arithmetic at all, rather than arithmetic that quietly drops or,
// worse, keeps claiming the fact.
int main() {
  const precept::nonzero<int> left = *precept::nonzero<int>::try_from(3);
  const precept::nonzero<int> right = *precept::nonzero<int>::try_from(3);
  const precept::nonzero<int> difference = left - right;
  return difference.value();
}
