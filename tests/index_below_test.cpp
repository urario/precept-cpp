// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/index_below.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace {

TEST(IndexBelowTest, AcceptsZeroAndTheLastIndex) {
  const auto first = precept::index_below<8>::try_from(0);
  const auto last = precept::index_below<8>::try_from(7);

  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(last.has_value());
  EXPECT_EQ(first->value(), 0U);
  EXPECT_EQ(last->value(), 7U);
}

TEST(IndexBelowTest, RejectsTheBoundAndTheNextRepresentableIndex) {
  EXPECT_FALSE(precept::index_below<8>::try_from(8).has_value());
  EXPECT_FALSE(precept::index_below<8>::try_from(9).has_value());
}

TEST(IndexBelowTest, HandlesTheSmallestNonEmptyBound) {
  const auto only = precept::index_below<1>::try_from(0);

  ASSERT_TRUE(only.has_value());
  EXPECT_EQ(only->value(), 0U);
  EXPECT_FALSE(precept::index_below<1>::try_from(1).has_value());
}

TEST(IndexBelowTest, TreatsZeroAsAnEmptyBound) {
  EXPECT_FALSE(precept::index_below<0>::try_from(0).has_value());
  EXPECT_FALSE(precept::index_below<0>::try_from(1).has_value());
  EXPECT_FALSE(precept::index_below<0>::try_from(static_cast<std::size_t>(-1)).has_value());
}

TEST(IndexBelowTest, CopyPreservesTheValidatedValue) {
  const auto original = precept::index_below<8>::try_from(6);
  ASSERT_TRUE(original.has_value());

  precept::index_below<8> copy = *original;
  const precept::index_below<8> preserved = copy;
  copy = *precept::index_below<8>::try_from(2);

  EXPECT_EQ(copy.value(), 2U);
  EXPECT_EQ(preserved.value(), 6U);
  EXPECT_EQ(original->value(), 6U);
}

// Fixed protocol field: validation happens at the input boundary and the same fact is reused by
// two deeper operations. The protocol owns exactly sixteen numbered fields, so the bound is stable.
namespace protocol {

using field_index = precept::index_below<16>;

constexpr std::size_t field_offset(field_index index) noexcept { return index.value() * 2U; }

constexpr bool is_control_field(field_index index) noexcept { return index.value() < 4U; }

struct decoded_field {
  std::size_t offset;
  bool control;
};

std::optional<decoded_field> decode_field(std::size_t raw_index) noexcept {
  const auto index = field_index::try_from(raw_index);
  if (!index) {
    return std::nullopt;
  }

  return decoded_field{field_offset(*index), is_control_field(*index)};
}

} // namespace protocol

TEST(IndexBelowUsageTest, ReusesAProtocolBoundAcrossCalls) {
  const auto decoded = protocol::decode_field(3);
  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(decoded->offset, 6U);
  EXPECT_TRUE(decoded->control);
  EXPECT_FALSE(protocol::decode_field(16).has_value());
}

// Fixed lookup table: the checked value composes with both array and fixed-extent span, but the
// standard containers still require the explicit scalar observation at the subscript operation.
namespace lookup {

using opcode_index = precept::index_below<4>;

constexpr std::array<std::string_view, 4> opcode_names{"load", "store", "add", "halt"};

std::string_view name(std::span<const std::string_view, 4> names, opcode_index index) noexcept {
  return names[index.value()];
}

std::optional<std::string_view> decode(std::size_t raw_index) noexcept {
  const auto index = opcode_index::try_from(raw_index);
  if (!index) {
    return std::nullopt;
  }

  return name(opcode_names, *index);
}

std::optional<std::string_view> decode_with_local_check(std::size_t raw_index) noexcept {
  if (raw_index >= opcode_names.size()) {
    return std::nullopt;
  }

  return opcode_names[raw_index];
}

} // namespace lookup

TEST(IndexBelowUsageTest, ComposesWithFixedExtentArrayAndSpan) {
  EXPECT_EQ(lookup::decode(2), "add");
  EXPECT_FALSE(lookup::decode(4).has_value());
}

TEST(IndexBelowUsageTest, MatchesALocalBoundsCheckWhenTheFactIsConsumedImmediately) {
  for (std::size_t index = 0; index <= lookup::opcode_names.size(); ++index) {
    EXPECT_EQ(lookup::decode(index), lookup::decode_with_local_check(index));
  }
}

TEST(IndexBelowUsageTest, LeavesOrdinaryLoopIndicesAsRawSizeValues) {
  std::string joined;
  for (std::size_t index = 0; index < lookup::opcode_names.size(); ++index) {
    joined += lookup::opcode_names[index];
  }

  EXPECT_EQ(joined, "loadstoreaddhalt");
}

} // namespace
