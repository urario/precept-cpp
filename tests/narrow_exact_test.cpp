// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/narrow_exact.hpp>

#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace {

TEST(NarrowExactTest, KeepsAValueOfTheSameWidth) {
  constexpr auto largest = std::numeric_limits<std::uint32_t>::max();

  EXPECT_EQ(precept::narrow_exact<std::uint32_t>(std::uint32_t{7}), 7U);
  EXPECT_EQ(precept::narrow_exact<std::uint32_t>(largest), largest);
  EXPECT_EQ(precept::narrow_exact<std::int32_t>(std::int32_t{-7}), -7);
}

TEST(NarrowExactTest, WideningAlwaysSucceeds) {
  EXPECT_EQ(precept::narrow_exact<std::int64_t>(std::numeric_limits<std::int8_t>::min()), -128);
  EXPECT_EQ(precept::narrow_exact<std::int64_t>(std::numeric_limits<std::uint32_t>::max()),
            4294967295);
  EXPECT_EQ(precept::narrow_exact<std::uint64_t>(std::uint8_t{255}), 255U);
}

TEST(NarrowExactTest, NarrowsAValueThatFits) {
  EXPECT_EQ(precept::narrow_exact<std::uint16_t>(std::size_t{4096}), 4096U);
  EXPECT_EQ(precept::narrow_exact<std::int8_t>(std::int64_t{-128}), -128);
}

TEST(NarrowExactTest, RejectsAValueAboveTheDestinationMaximum) {
  constexpr auto one_past_max = std::size_t{std::numeric_limits<std::uint16_t>::max()} + 1U;

  EXPECT_EQ(precept::narrow_exact<std::uint16_t>(one_past_max), std::nullopt);
  EXPECT_EQ(precept::narrow_exact<std::uint16_t>(std::numeric_limits<std::size_t>::max()),
            std::nullopt);
  EXPECT_EQ(precept::narrow_exact<std::int8_t>(std::int64_t{128}), std::nullopt);
}

TEST(NarrowExactTest, RejectsAValueBelowTheDestinationMinimum) {
  EXPECT_EQ(precept::narrow_exact<std::int8_t>(std::int64_t{-129}), std::nullopt);
  EXPECT_EQ(precept::narrow_exact<std::int32_t>(std::numeric_limits<std::int64_t>::min()),
            std::nullopt);
}

// The signed-to-unsigned direction is where a bare `static_cast` is most quietly wrong: it turns
// -1 into the destination's maximum instead of failing.
TEST(NarrowExactTest, RejectsANegativeValueForAnUnsignedDestination) {
  EXPECT_EQ(precept::narrow_exact<std::uint32_t>(std::int64_t{-1}), std::nullopt);
  EXPECT_EQ(precept::narrow_exact<std::uint64_t>(std::int64_t{-1}), std::nullopt);
  EXPECT_EQ(precept::narrow_exact<std::uint32_t>(std::numeric_limits<std::int64_t>::min()),
            std::nullopt);
  EXPECT_EQ(precept::narrow_exact<std::uint32_t>(std::int64_t{0}), 0U);
}

TEST(NarrowExactTest, ComparesAcrossSignednessAtTheBoundary) {
  constexpr auto uint32_max = std::numeric_limits<std::uint32_t>::max();
  constexpr auto int32_max = std::numeric_limits<std::int32_t>::max();

  EXPECT_EQ(precept::narrow_exact<std::uint32_t>(std::int64_t{uint32_max}), uint32_max);
  EXPECT_EQ(precept::narrow_exact<std::uint32_t>(std::int64_t{uint32_max} + 1), std::nullopt);
  EXPECT_EQ(precept::narrow_exact<std::int32_t>(std::uint64_t{int32_max}), int32_max);
  EXPECT_EQ(precept::narrow_exact<std::int32_t>(std::uint64_t{int32_max} + 1U), std::nullopt);
  EXPECT_EQ(precept::narrow_exact<std::int32_t>(std::numeric_limits<std::uint64_t>::max()),
            std::nullopt);
}

// Representative call site: a protocol field is narrower than the in-memory size type. The wire
// width is stated once, at the only place the payload size stops being a `std::size_t`.
namespace wire {

std::optional<std::array<std::byte, 2>>
encode_length_prefix(std::span<const std::byte> payload) noexcept {
  const auto wire_size = precept::narrow_exact<std::uint16_t>(payload.size());
  if (!wire_size) {
    return std::nullopt;
  }

  const auto high = static_cast<unsigned char>(*wire_size >> 8U);
  const auto low = static_cast<unsigned char>(*wire_size & 0xFFU);
  return std::array{std::byte{high}, std::byte{low}};
}

} // namespace wire

TEST(NarrowExactUsageTest, EncodesAPayloadSizeIntoAProtocolField) {
  const std::vector<std::byte> payload(258);
  const auto prefix = wire::encode_length_prefix(payload);

  ASSERT_TRUE(prefix.has_value());
  EXPECT_EQ((*prefix)[0], std::byte{1});
  EXPECT_EQ((*prefix)[1], std::byte{2});

  const std::vector<std::byte> too_long(std::size_t{std::numeric_limits<std::uint16_t>::max()} +
                                        1U);
  EXPECT_EQ(wire::encode_length_prefix(too_long), std::nullopt);
}

// Representative call site: an external signed value becomes an unsigned domain quantity. Both
// failure modes — negative and too large — are one rule here, not two guards.
//
// The standard spelling of the same rule is kept beside it: this experiment exists to compare the
// two, so the comparison is checked rather than asserted in prose.
namespace config {

std::optional<std::uint32_t> worker_count(std::int64_t requested) noexcept {
  return precept::narrow_exact<std::uint32_t>(requested);
}

std::optional<std::uint32_t> worker_count_standard(std::int64_t requested) noexcept {
  if (!std::in_range<std::uint32_t>(requested)) {
    return std::nullopt;
  }

  return static_cast<std::uint32_t>(requested);
}

} // namespace config

TEST(NarrowExactUsageTest, AcceptsAnExternalSignedValueAsAnUnsignedCount) {
  EXPECT_EQ(config::worker_count(4), 4U);
  EXPECT_EQ(config::worker_count(-1), std::nullopt);
  EXPECT_EQ(config::worker_count(std::int64_t{1} << 40), std::nullopt);
}

TEST(NarrowExactUsageTest, AgreesWithTheStandardSpellingOfTheSameRule) {
  constexpr std::int64_t inputs[] = {
      std::numeric_limits<std::int64_t>::min(), -4294967296, -1, 0, 1, 4294967295, 4294967296,
      std::numeric_limits<std::int64_t>::max()};

  for (const std::int64_t input : inputs) {
    EXPECT_EQ(config::worker_count(input), config::worker_count_standard(input)) << input;
  }
}

// Representative call site: an unsigned internal slot becomes the non-negative handle of a
// C-style API in which negative values are error codes.
namespace registry {

std::int32_t to_handle(std::uint64_t slot) noexcept {
  const auto handle = precept::narrow_exact<std::int32_t>(slot);
  return handle ? *handle : -1;
}

} // namespace registry

TEST(NarrowExactUsageTest, ConvertsAnUnsignedSlotIntoASignedHandle) {
  EXPECT_EQ(registry::to_handle(0), 0);
  EXPECT_EQ(registry::to_handle(17), 17);
  EXPECT_EQ(registry::to_handle(std::uint64_t{std::numeric_limits<std::int32_t>::max()}),
            std::numeric_limits<std::int32_t>::max());
  EXPECT_EQ(registry::to_handle(std::uint64_t{std::numeric_limits<std::int32_t>::max()} + 1U), -1);
}

// Representative call site outside serialization: a duration in the caller's own units becomes the
// `int` millisecond timeout of a poll-style API. Truncating or clamping here would silently change
// how long the caller waits, so a duration that does not fit is refused.
namespace scheduler {

std::optional<int> poll_timeout(std::chrono::milliseconds duration) noexcept {
  return precept::narrow_exact<int>(duration.count());
}

} // namespace scheduler

TEST(NarrowExactUsageTest, ConvertsADurationIntoAPollTimeout) {
  using namespace std::chrono_literals;

  EXPECT_EQ(scheduler::poll_timeout(250ms), 250);
  EXPECT_EQ(scheduler::poll_timeout(-1ms), -1);
  EXPECT_EQ(scheduler::poll_timeout(std::chrono::hours{24 * 365}), std::nullopt);
}

} // namespace
