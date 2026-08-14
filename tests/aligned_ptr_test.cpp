// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/aligned_ptr.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>

namespace {

TEST(AlignedPtrTest, PreservesAnAlignedPointer) {
  alignas(64) std::array<int, 2> values{};

  const auto result = precept::aligned_ptr<int, 64>::try_from(values.data());

  ASSERT_TRUE(result);
  EXPECT_EQ(result->get(), values.data());
}

TEST(AlignedPtrTest, ValidatesAConstPointerWithoutChangingQualification) {
  alignas(64) const std::array<int, 2> values{};

  const auto result = precept::aligned_ptr<const int, 64>::try_from(values.data());

  ASSERT_TRUE(result);
  EXPECT_EQ(result->get(), values.data());
}

TEST(AlignedPtrTest, RejectsALiveObjectAtAnInsufficientlyAlignedAddress) {
  alignas(64) std::array<int, 2> values{};

  const auto result = precept::aligned_ptr<int, 64>::try_from(&values[1]);

  EXPECT_FALSE(result);
}

TEST(AlignedPtrTest, RepresentsNullAsAValidPointerState) {
  const auto result = precept::aligned_ptr<int, 64>::try_from(nullptr);

  ASSERT_TRUE(result);
  EXPECT_EQ(result->get(), nullptr);
}

TEST(AlignedPtrTest, ImplicitlyPreservesConstAndWeakerAlignment) {
  alignas(64) int value = 42;
  const auto source = precept::aligned_ptr<int, 64>::try_from(&value);
  ASSERT_TRUE(source);

  const precept::aligned_ptr<const int, 32> target = *source;

  EXPECT_EQ(target.get(), &value);
  EXPECT_EQ(*target.get(), 42);
}

TEST(AlignedPtrTest, AcceptsTheNaturalAlignmentOfTheObjectType) {
  int value = 7;

  const auto result = precept::aligned_ptr<int, alignof(int)>::try_from(&value);

  ASSERT_TRUE(result);
  EXPECT_EQ(result->get(), &value);
}

} // namespace
