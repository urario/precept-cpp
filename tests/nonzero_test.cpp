// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/nonzero.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string_view>

namespace {

TEST(NonzeroTest, RejectsZero) {
  EXPECT_FALSE(precept::nonzero<int>::try_from(0).has_value());
  EXPECT_FALSE(precept::nonzero<unsigned int>::try_from(0U).has_value());
  EXPECT_FALSE(precept::nonzero<std::int8_t>::try_from(0).has_value());
  EXPECT_FALSE(precept::nonzero<char>::try_from('\0').has_value());
}

TEST(NonzeroTest, PreservesAPositiveValue) {
  const auto value = precept::nonzero<int>::try_from(42);

  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value->value(), 42);
}

TEST(NonzeroTest, PreservesANegativeValue) {
  const auto value = precept::nonzero<int>::try_from(-42);

  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value->value(), -42);
}

TEST(NonzeroTest, PreservesUnsignedAndExtremeValues) {
  constexpr auto unsigned_max = std::numeric_limits<unsigned int>::max();
  constexpr auto signed_min = std::numeric_limits<int>::min();

  const auto largest = precept::nonzero<unsigned int>::try_from(unsigned_max);
  const auto smallest = precept::nonzero<int>::try_from(signed_min);
  const auto one = precept::nonzero<std::size_t>::try_from(1);

  ASSERT_TRUE(largest.has_value());
  ASSERT_TRUE(smallest.has_value());
  ASSERT_TRUE(one.has_value());
  EXPECT_EQ(largest->value(), unsigned_max);
  EXPECT_EQ(smallest->value(), signed_min);
  EXPECT_EQ(one->value(), 1U);
}

// Non-zero is the whole guarantee. It is necessary for signed division but not sufficient, so a
// validated -1 divisor is still a divisor a caller must think about.
TEST(NonzeroTest, AcceptsDivisorsThatSignedDivisionStillConstrains) {
  const auto minus_one = precept::nonzero<int>::try_from(-1);

  ASSERT_TRUE(minus_one.has_value());
  EXPECT_EQ(minus_one->value(), -1);
}

TEST(NonzeroTest, CopyPreservesTheValidatedValue) {
  const auto original = precept::nonzero<int>::try_from(7);
  ASSERT_TRUE(original.has_value());

  precept::nonzero<int> copy = *original;
  const precept::nonzero<int> replaced = copy;
  copy = *precept::nonzero<int>::try_from(-3);

  EXPECT_EQ(copy.value(), -3);
  EXPECT_EQ(replaced.value(), 7);
  EXPECT_EQ(original->value(), 7);
}

// Representative call site: a divisor validated at a public boundary and reused by a helper that
// is never reached with an unvalidated value.
namespace ratio {

using divisor = precept::nonzero<std::uint32_t>;

std::uint32_t percentage_of(std::uint32_t part, divisor whole) noexcept {
  return part * 100U / whole.value();
}

std::optional<std::uint32_t> completion_percentage(std::uint32_t done,
                                                   std::uint32_t total) noexcept {
  const auto whole = divisor::try_from(total);
  if (!whole) {
    return std::nullopt;
  }

  return percentage_of(done, *whole);
}

} // namespace ratio

TEST(NonzeroUsageTest, ReusesADivisorValidatedAtThePublicBoundary) {
  EXPECT_FALSE(ratio::completion_percentage(3, 0).has_value());
  EXPECT_EQ(ratio::completion_percentage(3, 4), 75U);
}

// Representative call site with a different meaning: zero is the wire encoding for "no node", not
// an arithmetic hazard. The decode boundary rejects it once and the lookup layer below never sees
// it. The lookup still needs its own range check, because that is a different fact.
namespace graph {

using node_id = precept::nonzero<std::uint32_t>;

std::optional<node_id> decode_node(std::uint32_t raw) noexcept { return node_id::try_from(raw); }

std::string_view node_name(std::span<const std::string_view> table, node_id id) noexcept {
  const std::size_t index = id.value() - 1U;
  return index < table.size() ? table[index] : std::string_view{};
}

} // namespace graph

TEST(NonzeroUsageTest, KeepsTheAbsentSentinelOutOfTheLookupLayer) {
  constexpr std::string_view names[] = {"root", "left", "right"};

  EXPECT_FALSE(graph::decode_node(0).has_value());

  const auto second = graph::decode_node(2);
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(graph::node_name(names, *second), "left");

  const auto out_of_range = graph::decode_node(9);
  ASSERT_TRUE(out_of_range.has_value());
  EXPECT_TRUE(graph::node_name(names, *out_of_range).empty());
}

} // namespace
