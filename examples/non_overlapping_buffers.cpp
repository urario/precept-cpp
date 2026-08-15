// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Comparing a one-shot operation, a reusable pair, and a domain-specific three-buffer relation.

#include <precept/non_overlapping.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <span>

namespace {

using separated_io = precept::non_overlapping_spans<std::byte, const std::byte>;

// The relation has no useful lifetime beyond this operation. The carrier is validated and
// immediately unwrapped, so callers are better served by an operation that owns the failure
// boundary than by receiving the carrier themselves.
bool copy_chunk(std::span<std::byte> destination, std::span<const std::byte> source) noexcept {
  if (destination.size() != source.size()) {
    return false;
  }

  const auto separated = precept::checked_non_overlapping(destination, source);
  if (!separated) {
    return false;
  }

  std::copy(separated->second().begin(), separated->second().end(), separated->first().begin());
  return true;
}

void stage_a(separated_io buffers) noexcept {
  const std::size_t count = (std::min)(buffers.first().size(), buffers.second().size());
  for (std::size_t index = 0; index < count; ++index) {
    buffers.first()[index] = buffers.second()[index];
  }
}

void stage_b(separated_io buffers) noexcept {
  for (std::byte& value : buffers.first()) {
    value ^= std::byte{0x01};
  }
}

void stage_c(separated_io buffers) noexcept {
  const std::size_t count = (std::min)(buffers.first().size(), buffers.second().size());
  for (std::size_t index = 0; index < count; ++index) {
    buffers.first()[index] ^= buffers.second()[index];
  }
}

// Here the same validated pair crosses three call boundaries and is consumed three times.
bool process_in_stages(std::span<std::byte> output, std::span<const std::byte> input) noexcept {
  const auto separated = precept::checked_non_overlapping(output, input);
  if (!separated) {
    return false;
  }

  stage_a(*separated);
  stage_b(*separated);
  stage_c(*separated);
  return true;
}

// Pairwise non-overlap is necessary here, but the downstream contract is the role-bearing set of
// input, output, and scratch buffers. A domain object is clearer than passing three pair carriers
// that repeat each span and still do not name the buffer roles.
class processing_buffers {
public:
  [[nodiscard]] static std::optional<processing_buffers>
  try_from(std::span<const std::byte> input, std::span<std::byte> output,
           std::span<std::byte> scratch) noexcept {
    if (!precept::checked_non_overlapping(input, output) ||
        !precept::checked_non_overlapping(input, scratch) ||
        !precept::checked_non_overlapping(output, scratch)) {
      return std::nullopt;
    }

    return processing_buffers(input, output, scratch);
  }

  [[nodiscard]] std::span<const std::byte> input() const noexcept { return input_; }
  [[nodiscard]] std::span<std::byte> output() const noexcept { return output_; }
  [[nodiscard]] std::span<std::byte> scratch() const noexcept { return scratch_; }

private:
  processing_buffers(std::span<const std::byte> input, std::span<std::byte> output,
                     std::span<std::byte> scratch) noexcept
      : input_(input), output_(output), scratch_(scratch) {}

  std::span<const std::byte> input_;
  std::span<std::byte> output_;
  std::span<std::byte> scratch_;
};

void phase_1(const processing_buffers& buffers) noexcept {
  const std::size_t count = (std::min)(buffers.input().size(), buffers.scratch().size());
  std::copy_n(buffers.input().begin(), count, buffers.scratch().begin());
}

void phase_2(const processing_buffers& buffers) noexcept {
  const std::size_t count = (std::min)(buffers.output().size(), buffers.scratch().size());
  std::copy_n(buffers.scratch().begin(), count, buffers.output().begin());
}

void phase_3(const processing_buffers& buffers) noexcept {
  std::fill(buffers.scratch().begin(), buffers.scratch().end(), std::byte{});
}

} // namespace

int main() {
  constexpr std::array source = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};
  std::array<std::byte, source.size()> one_shot{};
  std::array<std::byte, source.size()> staged{};
  std::array<std::byte, source.size()> output{};
  std::array<std::byte, source.size()> scratch{};

  if (!copy_chunk(one_shot, source) || one_shot != source) {
    return 1;
  }

  std::array overlap_storage = {std::byte{1}, std::byte{2}, std::byte{3},
                                std::byte{4}, std::byte{5}, std::byte{6}};
  const auto before_failed_copy = overlap_storage;
  std::span<std::byte> overlap{overlap_storage};
  if (copy_chunk(overlap.first(4), std::span<const std::byte>{overlap.subspan(2, 4)}) ||
      overlap_storage != before_failed_copy) {
    return 2;
  }

  if (!process_in_stages(staged, source)) {
    return 3;
  }

  const auto buffers = processing_buffers::try_from(source, output, scratch);
  if (!buffers) {
    return 4;
  }

  phase_1(*buffers);
  phase_2(*buffers);
  phase_3(*buffers);
  if (output != source ||
      !std::ranges::all_of(scratch, [](std::byte value) { return value == std::byte{}; })) {
    return 5;
  }

  return 0;
}
