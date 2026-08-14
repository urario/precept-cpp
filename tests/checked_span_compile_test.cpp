// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/span/checked_span.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace {

template <std::size_t N, class Source>
concept accepts_checked_span = requires(Source source) { precept::checked_span<N>(source); };

static_assert(accepts_checked_span<4, std::span<int>>);
static_assert(accepts_checked_span<4, std::span<int, 4>>);
static_assert(!accepts_checked_span<4, std::span<int, 3>>);
static_assert(!accepts_checked_span<4, std::span<int, 5>>);
static_assert(!accepts_checked_span<std::dynamic_extent, std::span<int>>);

static_assert(!accepts_checked_span<4, std::array<int, 4>>);
static_assert(!accepts_checked_span<4, std::vector<int>>);

static_assert(std::same_as<decltype(precept::checked_span<4>(std::declval<std::span<int>>())),
                           std::optional<std::span<int, 4>>>);
static_assert(std::same_as<decltype(precept::checked_span<4>(std::declval<std::span<const int>>())),
                           std::optional<std::span<const int, 4>>>);
static_assert(std::same_as<decltype(precept::checked_span<4>(std::declval<std::span<int, 4>>())),
                           std::optional<std::span<int, 4>>>);

static_assert(noexcept(precept::checked_span<4>(std::declval<std::span<int>>())));

constexpr bool supports_constant_evaluation() {
  int values[] = {1, 2, 3, 4};
  auto result = precept::checked_span<4>(std::span<int>{values});
  return result && result->data() == values && (*result)[2] == 3;
}

constexpr bool supports_zero_extent() {
  std::span<int> empty;
  auto result = precept::checked_span<0>(empty);
  return result && result->empty();
}

static_assert(supports_constant_evaluation());
static_assert(supports_zero_extent());

} // namespace
