// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/checked_span.hpp>

#include <gtest/gtest.h>

#include <array>
#include <span>
#include <vector>

namespace {

TEST(CheckedSpanTest, AcceptsExactDynamicArraySpanAndAliasesStorage) {
  std::array<int, 4> values = {1, 2, 3, 4};

  auto result = precept::checked_span<4>(std::span<int>{values});

  ASSERT_TRUE(result);
  EXPECT_EQ(result->size(), 4U);
  EXPECT_EQ(result->data(), values.data());
  (*result)[1] = 9;
  EXPECT_EQ(values[1], 9);
}

TEST(CheckedSpanTest, RejectsShorterAndLongerDynamicSpansWithoutChangingTheSource) {
  std::array<int, 5> values = {1, 2, 3, 4, 5};
  const auto original = values;
  std::span<int> source{values};

  auto shorter = precept::checked_span<4>(source.first(3));
  auto longer = precept::checked_span<4>(source);

  EXPECT_FALSE(shorter);
  EXPECT_FALSE(longer);
  EXPECT_EQ(values, original);
}

TEST(CheckedSpanTest, AcceptsVectorThroughExplicitSpanConversion) {
  std::vector<int> values = {2, 4, 6, 8};

  auto result = precept::checked_span<4>(std::span<int>{values});

  ASSERT_TRUE(result);
  EXPECT_EQ(result->data(), values.data());
  EXPECT_EQ(result->back(), 8);
}

TEST(CheckedSpanTest, PreservesConstElementType) {
  const std::vector<int> values = {3, 6, 9};

  auto result = precept::checked_span<3>(std::span<const int>{values});

  ASSERT_TRUE(result);
  EXPECT_EQ(result->data(), values.data());
  EXPECT_EQ(result->front(), 3);
}

TEST(CheckedSpanTest, AcceptsFixedEqualSpanWithAnOptionalResult) {
  int values[] = {5, 6, 7, 8};

  auto result = precept::checked_span<4>(std::span<int, 4>{values});

  ASSERT_TRUE(result);
  EXPECT_EQ(result->data(), values);
}

TEST(CheckedSpanTest, AcceptsEmptyDynamicAndFixedSpansForZeroExtent) {
  std::span<int> dynamic_empty;
  std::span<int, 0> fixed_empty;

  auto dynamic_result = precept::checked_span<0>(dynamic_empty);
  auto fixed_result = precept::checked_span<0>(fixed_empty);

  ASSERT_TRUE(dynamic_result);
  EXPECT_TRUE(dynamic_result->empty());
  ASSERT_TRUE(fixed_result);
  EXPECT_TRUE(fixed_result->empty());
}

} // namespace
