// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/non_empty_span.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace {

TEST(AtLeastSpanTest, AcceptsExactAndGreaterDynamicSizes) {
  std::array<int, 5> values = {1, 2, 3, 4, 5};

  auto exact = precept::at_least_span<int, 4>::try_from(std::span<int>{values}.first(4));
  auto greater = precept::at_least_span<int, 4>::try_from(std::span<int>{values});

  ASSERT_TRUE(exact);
  EXPECT_EQ(exact->size(), 4U);
  ASSERT_TRUE(greater);
  EXPECT_EQ(greater->size(), 5U);
}

TEST(AtLeastSpanTest, RejectsInsufficientDynamicSizeWithoutChangingTheSource) {
  std::array<int, 3> values = {1, 2, 3};

  auto result = precept::at_least_span<int, 4>::try_from(std::span<int>{values});

  EXPECT_FALSE(result);
  EXPECT_EQ(values, (std::array<int, 3>{1, 2, 3}));
}

TEST(AtLeastSpanTest, SplitsGuaranteedPrefixAndRest) {
  int values[] = {10, 20, 30, 40, 50, 60};
  precept::at_least_span<int, 4> view = std::span<int, 6>{values};

  const auto prefix = view.prefix();
  const auto rest = view.rest();

  EXPECT_EQ(prefix.size(), 4U);
  EXPECT_EQ(prefix.front(), 10);
  EXPECT_EQ(prefix.back(), 40);
  ASSERT_EQ(rest.size(), 2U);
  EXPECT_EQ(rest[0], 50);
  EXPECT_EQ(rest[1], 60);
}

TEST(AtLeastSpanTest, ExactSizeHasAnEmptyRest) {
  std::array<int, 4> values = {1, 2, 3, 4};
  precept::at_least_span<int, 4> view = std::span<int, 4>{values};

  EXPECT_EQ(view.prefix().data(), values.data());
  EXPECT_TRUE(view.rest().empty());
}

TEST(AtLeastSpanTest, ProvidesStandardRangeAndElementAccess) {
  std::vector<int> values = {1, 2, 3, 4};
  auto result = precept::at_least_span<int, 2>::try_from(std::span<int>{values});
  ASSERT_TRUE(result);

  auto& view = *result;
  EXPECT_EQ(view.data(), values.data());
  EXPECT_EQ(view.size_bytes(), values.size() * sizeof(int));
  EXPECT_EQ(view.front(), 1);
  EXPECT_EQ(view.back(), 4);
  EXPECT_EQ(view[2], 3);

  int sum = 0;
  for (int value : view) {
    sum += value;
  }
  EXPECT_EQ(sum, 10);

  view.front() = 9;
  view.as_span().back() = 8;
  EXPECT_EQ(values.front(), 9);
  EXPECT_EQ(values.back(), 8);
}

TEST(AtLeastSpanTest, PreservesConstAndStrongerGuarantees) {
  int values[] = {1, 2, 3, 4};
  precept::at_least_span<int, 4> stronger = std::span<int, 4>{values};
  precept::at_least_span<const int, 2> weaker = stronger;

  EXPECT_EQ(weaker.size(), 4U);
  EXPECT_EQ(weaker.data(), values);
}

TEST(AtLeastSpanTest, IteratorDependsOnStorageRatherThanWrapperLifetime) {
  std::array<int, 3> values = {4, 5, 6};

  auto iterator = [&values] {
    auto view = precept::at_least_span<int, 2>::try_from(std::span<int>{values});
    return view->begin();
  }();

  EXPECT_EQ(*iterator, 4);
  EXPECT_EQ(*(iterator + 2), 6);
}

TEST(NonEmptySpanTest, RejectsEmptyDynamicSpan) {
  std::span<int> empty;
  EXPECT_FALSE(precept::non_empty_span<int>::try_from(empty));
}

TEST(NonEmptySpanTest, GuaranteesFrontAndBack) {
  std::vector<int> values = {7, 8, 9};
  auto result = precept::non_empty_span<int>::try_from(std::span<int>{values});

  ASSERT_TRUE(result);
  EXPECT_EQ(result->front(), 7);
  EXPECT_EQ(result->back(), 9);
}

} // namespace
