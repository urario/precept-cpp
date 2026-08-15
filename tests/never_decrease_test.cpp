// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/never_decrease.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

namespace {

TEST(NeverDecreaseTest, PreservesTheInitialValue) {
  const precept::never_decrease<int> value{-4};

  EXPECT_EQ(value.value(), -4);
}

TEST(NeverDecreaseTest, AcceptsEqualAndIncreasingUpdates) {
  precept::never_decrease<int> value{10};

  EXPECT_TRUE(value.try_update(10));
  EXPECT_EQ(value.value(), 10);
  EXPECT_TRUE(value.try_update(20));
  EXPECT_EQ(value.value(), 20);
}

TEST(NeverDecreaseTest, RejectsARegressionAndPreservesTheValue) {
  precept::never_decrease<int> value{20};

  EXPECT_FALSE(value.try_update(19));
  EXPECT_EQ(value.value(), 20);
}

TEST(NeverDecreaseTest, KeepsMultipleUpdatesNonDecreasing) {
  precept::never_decrease<std::size_t> value{0};

  EXPECT_TRUE(value.try_update(10));
  EXPECT_TRUE(value.try_update(10));
  EXPECT_TRUE(value.try_update(20));
  EXPECT_FALSE(value.try_update(15));
  EXPECT_TRUE(value.try_update(25));
  EXPECT_EQ(value.value(), 25U);
}

TEST(NeverDecreaseTest, AcceptsSignedValuesFromNegativeThroughZero) {
  precept::never_decrease<int> value{-10};

  EXPECT_TRUE(value.try_update(-5));
  EXPECT_TRUE(value.try_update(0));
  EXPECT_TRUE(value.try_update(5));
  EXPECT_FALSE(value.try_update(-1));
  EXPECT_EQ(value.value(), 5);
}

TEST(NeverDecreaseTest, AcceptsUnsignedBoundaryValuesWithoutIncrementing) {
  constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
  precept::never_decrease<std::uint64_t> value{maximum - 1U};

  EXPECT_TRUE(value.try_update(maximum));
  EXPECT_TRUE(value.try_update(maximum));
  EXPECT_FALSE(value.try_update(maximum - 1U));
  EXPECT_EQ(value.value(), maximum);
}

TEST(NeverDecreaseTest, CopiesHaveIndependentHistories) {
  precept::never_decrease<int> original{10};
  const precept::never_decrease<int> copy{original};

  ASSERT_TRUE(original.try_update(20));

  EXPECT_EQ(copy.value(), 10);
  EXPECT_EQ(original.value(), 20);
}

TEST(NeverDecreaseTest, MoveConstructionPreservesTheSourceHistory) {
  precept::never_decrease<int> source{10};

  const precept::never_decrease<int> destination{std::move(source)};

  EXPECT_EQ(source.value(), 10);
  EXPECT_EQ(destination.value(), 10);
  EXPECT_FALSE(source.try_update(9));
}

} // namespace
