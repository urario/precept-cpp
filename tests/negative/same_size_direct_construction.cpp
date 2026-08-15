// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/same_size.hpp>

#include <array>
#include <span>

int main() {
  std::array<int, 2> first{};
  std::array<int, 2> second{};
  const precept::same_size_pair<int, int> bypassed{std::span<int>{first}, std::span<int>{second}};
  return static_cast<int>(bypassed.size());
}
