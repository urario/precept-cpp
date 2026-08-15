// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/nonzero.hpp>

int main() {
  const precept::nonzero<int> bypassed{5};
  return bypassed.value();
}
