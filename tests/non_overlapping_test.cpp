// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/non_overlapping.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <span>
#include <utility>

namespace {

TEST(NonOverlappingTest, AcceptsClearlyDisjointSpans) {
  std::array<std::byte, 8> first{};
  std::array<std::byte, 8> second{};

  const auto checked = precept::checked_non_overlapping(std::span{first}, std::span{second});

  ASSERT_TRUE(checked.has_value());
  EXPECT_EQ(checked->first().data(), first.data());
  EXPECT_EQ(checked->second().data(), second.data());
}

TEST(NonOverlappingTest, RejectsClearlyOverlappingSpans) {
  std::array<std::byte, 16> storage{};
  std::span<std::byte> whole{storage};

  EXPECT_FALSE(
      precept::checked_non_overlapping(whole.subspan(2, 8), whole.subspan(6, 8)).has_value());
}

TEST(NonOverlappingTest, AcceptsTouchingSpans) {
  std::array<std::byte, 16> storage{};
  std::span<std::byte> whole{storage};

  const auto checked = precept::checked_non_overlapping(whole.first(8), whole.subspan(8, 8));

  ASSERT_TRUE(checked.has_value());
  EXPECT_EQ(checked->first().data() + checked->first().size(), checked->second().data());
}

TEST(NonOverlappingTest, AcceptsDisjointSubspansOfTheSameStorage) {
  std::array<std::byte, 24> storage{};
  std::span<std::byte> whole{storage};

  const auto checked = precept::checked_non_overlapping(whole.subspan(2, 4), whole.subspan(14, 6));

  ASSERT_TRUE(checked.has_value());
  EXPECT_EQ(checked->first().size(), 4U);
  EXPECT_EQ(checked->second().size(), 6U);
}

TEST(NonOverlappingTest, AcceptsEveryEmptyRangeCombination) {
  std::array<std::byte, 8> storage{};
  std::array<std::byte, 4> other{};
  std::span<std::byte> whole{storage};
  std::span<std::byte> empty{};

  EXPECT_TRUE(precept::checked_non_overlapping(empty, std::span{other}).has_value());
  EXPECT_TRUE(precept::checked_non_overlapping(std::span{other}, empty).has_value());
  EXPECT_TRUE(precept::checked_non_overlapping(empty, empty).has_value());
  EXPECT_TRUE(precept::checked_non_overlapping(whole.first(0), whole).has_value());
  EXPECT_TRUE(precept::checked_non_overlapping(whole, whole.last(0)).has_value());
}

TEST(NonOverlappingTest, AcceptsDifferentByteSizedElementTypesWhenDisjoint) {
  std::array<char, 4> first{};
  std::array<unsigned char, 4> second{};

  const auto checked =
      precept::checked_non_overlapping(std::span{first}, std::span<const unsigned char>{second});

  ASSERT_TRUE(checked.has_value());
  EXPECT_EQ(checked->first().data(), first.data());
  EXPECT_EQ(checked->second().data(), second.data());
}

TEST(NonOverlappingTest, CopyAndMovePreserveTheStoredSnapshots) {
  std::array<std::byte, 4> first{};
  std::array<std::byte, 4> second{};
  const auto original = precept::checked_non_overlapping(std::span{first}, std::span{second});
  ASSERT_TRUE(original.has_value());

  auto copy = *original;
  auto moved = std::move(copy);

  EXPECT_EQ(moved.first().data(), first.data());
  EXPECT_EQ(moved.second().data(), second.data());
  EXPECT_EQ(original->first().data(), first.data());
}

TEST(NonOverlappingTest, ObserversCannotRetargetTheStoredSnapshots) {
  std::array<std::byte, 4> first{};
  std::array<std::byte, 4> second{};
  std::array<std::byte, 4> replacement{};
  std::span<std::byte> first_view{first};
  const auto checked = precept::checked_non_overlapping(first_view, std::span{second});
  ASSERT_TRUE(checked.has_value());

  std::span<std::byte> observed = checked->first();
  observed = replacement;
  first_view = replacement;
  checked->first().front() = std::byte{42};

  EXPECT_EQ(checked->first().data(), first.data());
  EXPECT_EQ(checked->first().front(), std::byte{42});
}

} // namespace
