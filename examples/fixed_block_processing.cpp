// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Fixed-size block processing.
//
// Block ciphers, fixed-width records, and framed protocols all consume input in units of `N`
// elements. The usual loop carries a tail case for the remainder, and the decision of what a
// partial block means gets made in the middle of the loop. `block_span<T, N>` moves that decision
// to the boundary: either the input divides evenly and every iteration is a whole block, or the
// input never enters the loop.

#include <precept/span/block_span.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>

namespace {

constexpr std::size_t block_size = 16;

// Iteration yields `std::span<const std::byte, 16>`, so the block size is known statically inside
// the loop body as well as outside it.
void checksum_blocks(precept::block_span<const std::byte, block_size> blocks) {
  std::cout << "blocks: " << blocks.block_count() << '\n';

  for (std::span<const std::byte, block_size> block : blocks) {
    std::uint32_t sum = 0;
    for (const std::byte value : block) {
      sum += static_cast<std::uint32_t>(value);
    }
    std::cout << "  block checksum: " << sum << '\n';
  }
}

std::vector<std::byte> ciphertext(std::size_t size) {
  std::vector<std::byte> bytes(size, std::byte{0});
  for (std::size_t index = 0; index < size; ++index) {
    bytes[index] = static_cast<std::byte>(index);
  }
  return bytes;
}

} // namespace

int main() {
  const std::vector<std::byte> aligned = ciphertext(2 * block_size);
  if (auto blocks = precept::block_span<const std::byte, block_size>::try_from(
          std::span<const std::byte>{aligned})) {
    checksum_blocks(*blocks);
  }

  // A trailing partial block is a validation failure, not something the loop has to interpret.
  const std::vector<std::byte> ragged = ciphertext(block_size + 3);
  if (!precept::block_span<const std::byte, block_size>::try_from(
          std::span<const std::byte>{ragged})) {
    std::cout << "rejected " << ragged.size() << " bytes: not a whole number of blocks\n";
  }

  // Empty input is valid. It contains zero complete blocks, and the loop simply does not run.
  const std::vector<std::byte> nothing;
  if (auto blocks = precept::block_span<const std::byte, block_size>::try_from(
          std::span<const std::byte>{nothing})) {
    std::cout << "empty input is valid, blocks: " << blocks->block_count() << '\n';
  }

  // `size()` counts blocks; `as_span().size()` counts the underlying elements.
  if (auto blocks = precept::block_span<const std::byte, block_size>::try_from(
          std::span<const std::byte>{aligned})) {
    std::cout << "blocks: " << blocks->size() << ", elements: " << blocks->as_span().size() << '\n';
  }

  return 0;
}
