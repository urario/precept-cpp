// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/aligned_ptr.hpp>
#include <precept/narrow_exact.hpp>
#include <precept/nonzero.hpp>
#include <precept/set_once.hpp>
#include <precept/span/at_least_span.hpp>
#include <precept/span/block_span.hpp>
#include <precept/span/checked_span.hpp>
#include <precept/span/non_empty_span.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

int main() {
  std::array<std::byte, 16> storage{};
  std::span<const std::byte> input{storage};

  const auto minimum = precept::at_least_span<const std::byte, 8>::try_from(input);
  if (!minimum || minimum->prefix().size() != 8 || minimum->rest().size() != 8) {
    return 1;
  }

  const auto non_empty = precept::non_empty_span<const std::byte>::try_from(input);
  if (!non_empty || non_empty->front() != storage.front() || non_empty->back() != storage.back()) {
    return 2;
  }

  const auto exact = precept::checked_span<16>(input);
  if (!exact || exact->size() != storage.size()) {
    return 3;
  }

  const auto blocks = precept::block_span<const std::byte, 4>::try_from(input);
  if (!blocks || blocks->block_count() != 4 || blocks->size() != 4) {
    return 4;
  }

  for (const std::span<const std::byte, 4> block : *blocks) {
    if (block.size() != 4) {
      return 5;
    }
  }

  alignas(64) int value = 42;
  const auto aligned = precept::aligned_ptr<int, 64>::try_from(&value);
  if (!aligned || aligned->get() != &value) {
    return 6;
  }

  precept::set_once<int> configured;
  if (!configured.try_set(42) || configured.value() != 42 || configured.try_set(7)) {
    return 7;
  }

  const auto divisor = precept::nonzero<int>::try_from(7);
  if (!divisor || divisor->value() != 7 || precept::nonzero<int>::try_from(0)) {
    return 8;
  }

  const auto wire_size = precept::narrow_exact<std::uint16_t>(storage.size());
  if (!wire_size || *wire_size != 16 || precept::narrow_exact<std::uint32_t>(-1)) {
    return 9;
  }

  return 0;
}
