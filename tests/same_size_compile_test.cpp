// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/same_size.hpp>

#include <concepts>
#include <cstddef>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>

namespace {

template <class T, class U>
concept can_check_same_size = requires(std::span<T> first, std::span<U> second) {
  precept::checked_same_size(first, second);
};

template <class T, std::size_t E, class U, std::size_t F>
concept can_check_fixed_extent = requires(std::span<T, E> first, std::span<U, F> second) {
  precept::checked_same_size(first, second);
};

template <class T, class U, std::size_t E, std::size_t F>
concept can_check_with_explicit_element_types =
    requires(std::span<T, E> first, std::span<U, F> second) {
      precept::checked_same_size<T, U>(first, second);
    };

using mutable_pair = precept::same_size_pair<int, int>;
using qualified_pair = precept::same_size_pair<int, const float>;

static_assert(!std::default_initializable<mutable_pair>);
static_assert(!std::constructible_from<mutable_pair, std::span<int>, std::span<int>>);
static_assert(std::copy_constructible<mutable_pair>);
static_assert(std::movable<mutable_pair>);
static_assert(std::is_trivially_copyable_v<mutable_pair>);

static_assert(std::same_as<decltype(std::declval<const mutable_pair&>().first()), std::span<int>>);
static_assert(
    std::same_as<decltype(std::declval<const qualified_pair&>().second()), std::span<const float>>);
static_assert(std::same_as<decltype(std::declval<const qualified_pair&>().size()), std::size_t>);
static_assert(noexcept(std::declval<const mutable_pair&>().first()));
static_assert(noexcept(std::declval<const mutable_pair&>().second()));
static_assert(noexcept(std::declval<const mutable_pair&>().size()));

static_assert(can_check_same_size<int, int>);
static_assert(can_check_same_size<int, const float>);
static_assert(!can_check_fixed_extent<int, 2, int, 2>);
static_assert(!can_check_fixed_extent<int, 2, int, 3>);
static_assert(!can_check_with_explicit_element_types<int, int, 2, 2>);
static_assert(!can_check_with_explicit_element_types<int, int, 2, 3>);
static_assert(can_check_with_explicit_element_types<int, const int, std::dynamic_extent,
                                                    std::dynamic_extent>);

static_assert(std::same_as<decltype(precept::checked_same_size(
                               std::declval<std::span<int>>(), std::declval<std::span<const int>>())),
                           std::optional<precept::same_size_pair<int, const int>>>);
static_assert(noexcept(precept::checked_same_size(std::declval<std::span<int>>(),
                                                  std::declval<std::span<const int>>())));

constexpr bool supports_constant_evaluation() {
  int first[] = {1, 2, 3};
  const int second[] = {4, 5, 6};
  const auto result = precept::checked_same_size(std::span<int>{first}, std::span<const int>{second});
  return result && result->first().data() == first && result->second().data() == second &&
         result->size() == 3;
}

constexpr bool supports_empty_constant_evaluation() {
  const std::span<int> first;
  const std::span<const int> second;
  const auto result = precept::checked_same_size(first, second);
  return result && result->size() == 0;
}

static_assert(supports_constant_evaluation());
static_assert(supports_empty_constant_evaluation());

} // namespace
