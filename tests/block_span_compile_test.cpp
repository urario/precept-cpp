// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/block_span.hpp>

#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>

namespace {

using mutable_four = precept::block_span<int, 4>;
using const_four = precept::block_span<const int, 4>;
using mutable_two = precept::block_span<int, 2>;

template <class Target, class Source>
concept accepts_try_from = requires(Source source) { Target::try_from(source); };

template <std::size_t N>
concept valid_block_span = requires { typename precept::block_span<int, N>; };

static_assert(std::default_initializable<mutable_four>);
static_assert(std::copyable<mutable_four>);
static_assert(mutable_four::block_size == 4);

static_assert(std::convertible_to<std::span<int, 0>, mutable_four>);
static_assert(std::convertible_to<std::span<int, 4>, mutable_four>);
static_assert(std::convertible_to<std::span<int, 8>, mutable_four>);
static_assert(!std::constructible_from<mutable_four, std::span<int, 6>>);
static_assert(!std::constructible_from<mutable_four, std::span<int>>);
static_assert(accepts_try_from<mutable_four, std::span<int>>);
static_assert(!accepts_try_from<mutable_four, std::span<int, 4>>);
static_assert(!accepts_try_from<mutable_four, std::span<int, 6>>);

static_assert(std::convertible_to<mutable_four, const_four>);
static_assert(!std::convertible_to<const_four, mutable_four>);
static_assert(!std::constructible_from<mutable_two, mutable_four>);
static_assert(!std::constructible_from<mutable_four, mutable_two>);

static_assert(std::same_as<mutable_four::element_type, int>);
static_assert(std::same_as<mutable_four::block_type, std::span<int, 4>>);
static_assert(std::same_as<mutable_four::value_type, std::span<int, 4>>);
static_assert(std::same_as<mutable_four::reference, std::span<int, 4>>);
static_assert(std::same_as<std::iter_reference_t<mutable_four::iterator>, std::span<int, 4>>);
static_assert(std::same_as<decltype(std::declval<mutable_four>().as_span()), std::span<int>>);
static_assert(std::same_as<decltype(mutable_four::try_from(std::declval<std::span<int>>())),
                           std::optional<mutable_four>>);

static_assert(std::ranges::common_range<mutable_four>);
static_assert(std::ranges::sized_range<mutable_four>);
static_assert(std::ranges::random_access_range<mutable_four>);
static_assert(std::ranges::borrowed_range<mutable_four>);
static_assert(std::ranges::borrowed_range<const mutable_four>);
static_assert(!std::ranges::contiguous_range<mutable_four>);

static_assert(valid_block_span<1>);
static_assert(!valid_block_span<0>);
static_assert(!valid_block_span<std::dynamic_extent>);

static_assert(noexcept(mutable_four::try_from(std::declval<std::span<int>>())));
static_assert(noexcept(std::declval<mutable_four>().as_span()));
static_assert(noexcept(std::declval<mutable_four>().begin()));
static_assert(noexcept(std::declval<mutable_four>().end()));

constexpr bool supports_constant_evaluation() {
  int values[] = {1, 2, 3, 4, 5, 6, 7, 8};
  mutable_four blocks = std::span<int, 8>{values};
  const auto second = blocks[1];
  return blocks.size() == 2 && blocks.block_count() == 2 && !blocks.empty() &&
         blocks.as_span().size() == 8 && second.front() == 5 &&
         (blocks.end() - blocks.begin()) == 2 && blocks.begin()[1].back() == 8;
}

static_assert(supports_constant_evaluation());

} // namespace
