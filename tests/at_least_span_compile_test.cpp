// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/non_empty_span.hpp>

#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>

namespace {

using mutable_four = precept::at_least_span<int, 4>;
using const_four = precept::at_least_span<const int, 4>;
using mutable_eight = precept::at_least_span<int, 8>;

template <class Target, class Source>
concept accepts_try_from = requires(Source source) { Target::try_from(source); };

template <class T>
concept has_empty_member = requires(T value) { value.empty(); };

template <std::size_t N>
concept valid_at_least_span = requires { typename precept::at_least_span<int, N>; };

static_assert(mutable_four::minimum_size == 4);
static_assert(std::same_as<precept::non_empty_span<int>, precept::at_least_span<int, 1>>);
static_assert(!std::default_initializable<mutable_four>);
static_assert(std::copyable<mutable_four>);

static_assert(std::convertible_to<std::span<int, 4>, mutable_four>);
static_assert(std::convertible_to<std::span<int, 8>, mutable_four>);
static_assert(!std::constructible_from<mutable_four, std::span<int, 3>>);
static_assert(!std::constructible_from<mutable_four, std::span<int>>);
static_assert(accepts_try_from<mutable_four, std::span<int>>);
static_assert(!accepts_try_from<mutable_four, std::span<int, 4>>);
static_assert(!accepts_try_from<mutable_four, std::span<int, 3>>);
static_assert(accepts_try_from<const_four, std::span<int>>);
static_assert(!accepts_try_from<mutable_four, std::span<const int>>);

static_assert(std::convertible_to<mutable_eight, mutable_four>);
static_assert(std::convertible_to<mutable_four, const_four>);
static_assert(!std::convertible_to<mutable_four, mutable_eight>);
static_assert(!std::convertible_to<const_four, mutable_four>);
static_assert(std::convertible_to<mutable_four, std::span<int>>);
static_assert(std::convertible_to<mutable_four, std::span<const int>>);
static_assert(!std::convertible_to<mutable_four, std::span<int, 4>>);

static_assert(std::same_as<mutable_four::element_type, int>);
static_assert(std::same_as<mutable_four::value_type, int>);
static_assert(std::same_as<mutable_four::pointer, int*>);
static_assert(std::same_as<mutable_four::reference, int&>);
static_assert(std::same_as<decltype(std::declval<mutable_four>().as_span()), std::span<int>>);
static_assert(std::same_as<decltype(std::declval<mutable_four>().prefix()), std::span<int, 4>>);
static_assert(std::same_as<decltype(std::declval<mutable_four>().rest()), std::span<int>>);
static_assert(std::same_as<decltype(mutable_four::try_from(std::declval<std::span<int>>())),
                           std::optional<mutable_four>>);
static_assert(!has_empty_member<mutable_four>);

static_assert(std::ranges::contiguous_range<mutable_four>);
static_assert(std::ranges::sized_range<mutable_four>);
static_assert(std::ranges::borrowed_range<mutable_four>);
static_assert(std::ranges::borrowed_range<const mutable_four>);
static_assert(requires(mutable_four value) { std::ranges::empty(value); });

static_assert(valid_at_least_span<1>);
static_assert(!valid_at_least_span<0>);
static_assert(!valid_at_least_span<std::dynamic_extent>);

static_assert(noexcept(mutable_four::try_from(std::declval<std::span<int>>())));
static_assert(noexcept(std::declval<mutable_four>().prefix()));
static_assert(noexcept(std::declval<mutable_four>().rest()));
static_assert(noexcept(std::declval<mutable_four>().as_span()));

constexpr bool supports_constant_evaluation() {
  int values[] = {1, 2, 3, 4, 5};
  mutable_four view = std::span<int, 5>{values};
  return view.size() == 5 && view.prefix().size() == 4 && view.rest().size() == 1 &&
         view.front() == 1 && view.back() == 5 && view[2] == 3;
}

static_assert(supports_constant_evaluation());

} // namespace
