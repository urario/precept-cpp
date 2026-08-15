// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/same_size.hpp>

#include <gtest/gtest.h>

#include <array>
#include <span>
#include <utility>

namespace {

TEST(SameSizeTest, AcceptsEqualDynamicSpans) {
  std::array<int, 3> first{1, 2, 3};
  std::array<float, 3> second{4.0F, 5.0F, 6.0F};

  const auto checked = precept::checked_same_size(std::span<int>{first}, std::span<const float>{second});

  ASSERT_TRUE(checked.has_value());
  EXPECT_EQ(checked->first().size(), first.size());
  EXPECT_EQ(checked->second().size(), second.size());
  EXPECT_EQ(checked->size(), first.size());
  EXPECT_EQ(checked->first().data(), first.data());
  EXPECT_EQ(checked->second().data(), second.data());
}

TEST(SameSizeTest, RejectsUnequalDynamicSpans) {
  std::array<int, 2> first{};
  std::array<int, 3> second{};

  EXPECT_FALSE(precept::checked_same_size(std::span<int>{first}, std::span<int>{second}));
}

TEST(SameSizeTest, AcceptsTwoEmptySpans) {
  const std::span<int> first;
  const std::span<const float> second;

  const auto checked = precept::checked_same_size(first, second);

  ASSERT_TRUE(checked.has_value());
  EXPECT_TRUE(checked->first().empty());
  EXPECT_TRUE(checked->second().empty());
  EXPECT_EQ(checked->size(), 0U);
}

TEST(SameSizeTest, RejectsEmptyAndNonEmptySpans) {
  std::array<int, 1> values{};

  EXPECT_FALSE(precept::checked_same_size(std::span<int>{}, std::span<int>{values}));
  EXPECT_FALSE(precept::checked_same_size(std::span<int>{values}, std::span<int>{}));
}

TEST(SameSizeTest, SourceSpanReassignmentDoesNotRetargetSnapshots) {
  std::array<int, 2> first{1, 2};
  std::array<int, 2> second{3, 4};
  std::array<int, 2> replacement_first{5, 6};
  std::array<int, 2> replacement_second{7, 8};
  std::span<int> first_view{first};
  std::span<int> second_view{second};

  const auto checked = precept::checked_same_size(first_view, second_view);
  ASSERT_TRUE(checked.has_value());

  first_view = replacement_first;
  second_view = replacement_second;

  EXPECT_EQ(checked->first().data(), first.data());
  EXPECT_EQ(checked->second().data(), second.data());
  EXPECT_EQ(checked->first().size(), 2U);
  EXPECT_EQ(checked->second().size(), 2U);
}

TEST(SameSizeTest, CopyAndMovePreserveSnapshots) {
  std::array<int, 2> first{};
  std::array<const int, 2> second{};

  const auto original =
      precept::checked_same_size(std::span<int>{first}, std::span<const int>{second});
  ASSERT_TRUE(original.has_value());

  auto copy = *original;
  auto moved = std::move(copy);

  EXPECT_EQ(moved.first().data(), first.data());
  EXPECT_EQ(moved.second().data(), second.data());
  EXPECT_EQ(original->size(), moved.size());
}

TEST(SameSizeTest, MutableFirstSpanRemainsMutable) {
  std::array<int, 1> first{1};
  std::array<const float, 1> second{2.0F};

  const auto checked =
      precept::checked_same_size(std::span<int>{first}, std::span<const float>{second});
  ASSERT_TRUE(checked.has_value());

  checked->first().front() = 9;
  EXPECT_EQ(first.front(), 9);
  EXPECT_FLOAT_EQ(checked->second().front(), 2.0F);
}

} // namespace
