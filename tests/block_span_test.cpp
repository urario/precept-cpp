// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/block_span.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <ranges>
#include <span>
#include <vector>

namespace {

TEST(BlockSpanTest, DefaultAndDynamicEmptyInputsContainZeroBlocks) {
  precept::block_span<int, 4> default_empty;
  std::span<int> dynamic_empty;
  auto validated_empty = precept::block_span<int, 4>::try_from(dynamic_empty);

  EXPECT_TRUE(default_empty.empty());
  EXPECT_EQ(default_empty.size(), 0U);
  EXPECT_EQ(default_empty.begin(), default_empty.end());
  ASSERT_TRUE(validated_empty);
  EXPECT_TRUE(validated_empty->empty());
  EXPECT_TRUE(validated_empty->as_span().empty());
}

TEST(BlockSpanTest, AcceptsOneAndMultipleCompleteBlocks) {
  std::array<int, 12> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  std::span<int> source{values};

  auto one = precept::block_span<int, 4>::try_from(source.first(4));
  auto multiple = precept::block_span<int, 4>::try_from(source);

  ASSERT_TRUE(one);
  EXPECT_EQ(one->size(), 1U);
  ASSERT_TRUE(multiple);
  EXPECT_EQ(multiple->size(), 3U);
  EXPECT_EQ((*multiple)[0].front(), 1);
  EXPECT_EQ((*multiple)[1].front(), 5);
  EXPECT_EQ((*multiple)[2].back(), 12);
}

TEST(BlockSpanTest, RejectsRemainderWithoutChangingTheSource) {
  std::array<int, 5> values = {1, 2, 3, 4, 5};
  const auto original = values;

  auto result = precept::block_span<int, 4>::try_from(std::span<int>{values});

  EXPECT_FALSE(result);
  EXPECT_EQ(values, original);
}

TEST(BlockSpanTest, SupportsOneElementBlocks) {
  std::array<int, 3> values = {2, 4, 6};
  auto result = precept::block_span<int, 1>::try_from(std::span<int>{values});

  ASSERT_TRUE(result);
  EXPECT_EQ(result->size(), values.size());
  EXPECT_EQ((*result)[1].front(), 4);
}

TEST(BlockSpanTest, ReportsBlockAndElementCountsSeparately) {
  int values[] = {1, 2, 3, 4, 5, 6, 7, 8};
  precept::block_span<int, 4> blocks = std::span<int, 8>{values};

  EXPECT_EQ(blocks.size(), 2U);
  EXPECT_EQ(blocks.block_count(), blocks.size());
  EXPECT_EQ(std::ranges::size(blocks), blocks.size());
  EXPECT_EQ(blocks.as_span().size(), blocks.size() * blocks.block_size);
  EXPECT_EQ(blocks.as_span().data(), values);
}

TEST(BlockSpanTest, IteratesAndIndexesInBlocks) {
  std::vector<int> values = {1, 2, 3, 4, 5, 6, 7, 8};
  auto result = precept::block_span<int, 2>::try_from(std::span<int>{values});
  ASSERT_TRUE(result);

  std::size_t index = 0;
  for (std::span<int, 2> block : *result) {
    EXPECT_EQ(block.front(), values[index * 2]);
    block.back() *= 10;
    ++index;
  }

  EXPECT_EQ(index, result->size());
  EXPECT_EQ(values, (std::vector<int>{1, 20, 3, 40, 5, 60, 7, 80}));
  EXPECT_EQ((*(result->begin() + 2)).size(), 2U);
  EXPECT_EQ(result->begin()[3].front(), 7);
  EXPECT_LT(result->begin(), result->end());
}

TEST(BlockSpanTest, PreservesMutableToConstConversionAndShallowConstness) {
  int values[] = {1, 2, 3, 4};
  precept::block_span<int, 2> mutable_blocks = std::span<int, 4>{values};
  precept::block_span<const int, 2> read_only_blocks = mutable_blocks;
  const precept::block_span<int, 2> shallow_const = mutable_blocks;

  shallow_const[0].front() = 9;

  EXPECT_EQ(values[0], 9);
  EXPECT_EQ(read_only_blocks[0].front(), 9);
}

TEST(BlockSpanTest, IteratorDependsOnStorageRatherThanWrapperLifetime) {
  std::array<int, 4> values = {3, 6, 9, 12};

  auto iterator = [&values] {
    auto blocks = precept::block_span<int, 2>::try_from(std::span<int>{values});
    return blocks->begin();
  }();

  EXPECT_EQ((*iterator).front(), 3);
  EXPECT_EQ((*(iterator + 1)).back(), 12);
}

} // namespace
