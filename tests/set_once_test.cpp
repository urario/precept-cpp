// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/set_once.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

struct copy_observer {
  int value;
  int* copies;

  copy_observer(int initial, int& copy_count) : value(initial), copies(&copy_count) {}

  copy_observer(const copy_observer& source) : value(source.value), copies(source.copies) {
    ++*copies;
  }
};

struct move_observer {
  int value;
  int* moves;

  move_observer(int initial, int& move_count) : value(initial), moves(&move_count) {}
  move_observer(const move_observer&) = delete;
  move_observer& operator=(const move_observer&) = delete;

  move_observer(move_observer&& source) noexcept : value(source.value), moves(source.moves) {
    ++*moves;
    source.value = -1;
  }
};

struct construction_observer {
  int value;

  construction_observer(int initial, int& construction_count) : value(initial) {
    ++construction_count;
  }
};

struct conditionally_throwing {
  explicit conditionally_throwing(bool should_throw) {
    if (should_throw) {
      throw std::runtime_error("construction failed");
    }
  }
};

TEST(SetOnceTest, StartsUnsetAndSetsAnLvalueOnce) {
  precept::set_once<std::string> value;
  const std::string first = "first";

  EXPECT_FALSE(value.has_value());
  EXPECT_TRUE(value.try_set(first));
  EXPECT_TRUE(value.has_value());
  EXPECT_EQ(value.value(), first);
  EXPECT_FALSE(value.try_set(std::string{"second"}));
  EXPECT_EQ(value.value(), first);
}

TEST(SetOnceTest, DoesNotMoveFromAnArgumentWhenAlreadySet) {
  int moves = 0;
  precept::set_once<move_observer> value;
  move_observer first{1, moves};
  move_observer second{2, moves};

  EXPECT_TRUE(value.try_set(std::move(first)));
  EXPECT_EQ(moves, 1);
  EXPECT_FALSE(value.try_set(std::move(second)));
  EXPECT_EQ(moves, 1);
  EXPECT_EQ(second.value, 2);
  EXPECT_EQ(value.value().value, 1);
}

TEST(SetOnceTest, DoesNotCopyAnArgumentWhenAlreadySet) {
  int copies = 0;
  precept::set_once<copy_observer> value;
  const copy_observer first{1, copies};
  const copy_observer second{2, copies};

  EXPECT_TRUE(value.try_set(first));
  EXPECT_EQ(copies, 1);
  EXPECT_FALSE(value.try_set(second));
  EXPECT_EQ(copies, 1);
  EXPECT_EQ(second.value, 2);
  EXPECT_EQ(value.value().value, 1);
}

TEST(SetOnceTest, DoesNotConstructAnEmplacedValueWhenAlreadySet) {
  int constructions = 0;
  precept::set_once<construction_observer> value;

  EXPECT_TRUE(value.try_emplace(1, constructions));
  EXPECT_EQ(constructions, 1);
  EXPECT_FALSE(value.try_emplace(2, constructions));
  EXPECT_EQ(constructions, 1);
  EXPECT_EQ(value.value().value, 1);
}

TEST(SetOnceTest, RemainsUnsetWhenConstructionThrows) {
  precept::set_once<conditionally_throwing> value;

  EXPECT_THROW(static_cast<void>(value.try_emplace(true)), std::runtime_error);
  EXPECT_FALSE(value.has_value());
  EXPECT_TRUE(value.try_emplace(false));
  EXPECT_TRUE(value.has_value());
}

TEST(SetOnceTest, ThrowsWhenAnUnsetValueIsObserved) {
  const precept::set_once<int> value;

  EXPECT_THROW(static_cast<void>(value.value()), std::bad_optional_access);
}

TEST(SetOnceTest, CopyConstructionPreservesUnsetAndSetStates) {
  const precept::set_once<std::string> unset;
  const precept::set_once<std::string> unset_copy{unset};
  EXPECT_FALSE(unset_copy.has_value());

  precept::set_once<std::string> source;
  ASSERT_TRUE(source.try_emplace("configured"));
  const precept::set_once<std::string> copy{source};

  EXPECT_TRUE(copy.has_value());
  EXPECT_EQ(copy.value(), "configured");
}

TEST(SetOnceTest, MoveConstructionLeavesTheSourceSet) {
  precept::set_once<std::string> unset;
  const precept::set_once<std::string> moved_unset{std::move(unset)};
  EXPECT_FALSE(unset.has_value());
  EXPECT_FALSE(moved_unset.has_value());

  precept::set_once<std::string> source;
  ASSERT_TRUE(source.try_emplace("configured"));

  const precept::set_once<std::string> destination{std::move(source)};

  EXPECT_TRUE(destination.has_value());
  EXPECT_EQ(destination.value(), "configured");
  EXPECT_TRUE(source.has_value());
  EXPECT_FALSE(source.try_set(std::string{"replacement"}));
}

TEST(SetOnceTest, DoesNotClaimDeepImmutabilityOfTheContainedValue) {
  precept::set_once<std::unique_ptr<int>> value;
  ASSERT_TRUE(value.try_set(std::make_unique<int>(1)));

  *value.value() = 2;

  EXPECT_EQ(*value.value(), 2);
}

} // namespace
