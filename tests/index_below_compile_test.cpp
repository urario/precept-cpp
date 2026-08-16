// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/index_below.hpp>

#include <concepts>
#include <cstddef>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

template <std::size_t N>
concept valid_index_below = requires { typename precept::index_below<N>; };

using empty_index = precept::index_below<0>;
using four_index = precept::index_below<4>;
using single_index = precept::index_below<1>;
using small_index = precept::index_below<8>;
using large_index = precept::index_below<16>;
using maximum_bound_index = precept::index_below<std::numeric_limits<std::size_t>::max()>;

// Zero is an admitted bound. It describes an empty value set, so validation always fails rather
// than making generic fixed-extent code reject the type itself.
static_assert(valid_index_below<0>);
static_assert(valid_index_below<1>);
static_assert(valid_index_below<8>);

// The invariant has no bypass: no default state, no raw-index construction, and no conversion in.
static_assert(!std::default_initializable<empty_index>);
static_assert(!std::default_initializable<small_index>);
static_assert(!std::constructible_from<small_index, std::size_t>);
static_assert(!std::convertible_to<std::size_t, small_index>);
static_assert(!std::assignable_from<small_index&, std::size_t>);

// Copying preserves the value. Observation is explicit and has the ordinary scalar type.
static_assert(std::copyable<small_index>);
static_assert(std::is_trivially_copyable_v<small_index>);
static_assert(std::movable<small_index>);
static_assert(sizeof(small_index) == sizeof(std::size_t));
static_assert(!std::convertible_to<small_index, std::size_t>);
static_assert(std::same_as<decltype(std::declval<const small_index&>().value()), std::size_t>);
static_assert(noexcept(std::declval<const small_index&>().value()));
static_assert(std::same_as<decltype(small_index::try_from(0)), std::optional<small_index>>);
static_assert(noexcept(small_index::try_from(0)));

// A source whose bound is no greater than the destination already satisfies the destination fact.
// The conversion is implicit and preserves the validated scalar without another validation step.
static_assert(std::constructible_from<small_index, const four_index&>);
static_assert(std::constructible_from<large_index, const four_index&>);
static_assert(std::convertible_to<four_index, small_index>);
static_assert(std::convertible_to<four_index, large_index>);
static_assert(noexcept(small_index{std::declval<const four_index&>()}));
static_assert(noexcept(large_index{std::declval<const four_index&>()}));

// Strengthening is not a conversion. It remains rejected for both representative destinations.
static_assert(!std::constructible_from<four_index, small_index>);
static_assert(!std::constructible_from<four_index, large_index>);
static_assert(!std::convertible_to<large_index, small_index>);
static_assert(!std::convertible_to<large_index, four_index>);

// No bound arithmetic is needed even for the largest representable destination bound.
static_assert(std::convertible_to<four_index, maximum_bound_index>);

constexpr bool supports_constant_evaluation() {
  const auto source = *four_index::try_from(3);
  const small_index widened_to_eight = source;
  const large_index widened_to_sixteen = source;
  const maximum_bound_index widened_to_maximum = source;
  if (widened_to_eight.value() != 3 || widened_to_sixteen.value() != 3 ||
      widened_to_maximum.value() != 3) {
    return false;
  }

  const auto first = small_index::try_from(0);
  const auto last = small_index::try_from(7);
  if (!first || first->value() != 0 || !last || last->value() != 7) {
    return false;
  }

  if (small_index::try_from(8) || small_index::try_from(9)) {
    return false;
  }

  return !empty_index::try_from(0) && !single_index::try_from(1);
}

static_assert(supports_constant_evaluation());

inline constexpr small_index fixed_slot = *small_index::try_from(3);
static_assert(fixed_slot.value() == 3);

static_assert(!std::constructible_from<empty_index, small_index>);
static_assert(!std::convertible_to<small_index, empty_index>);

// Equal bounds do not distinguish domains. A domain that must reject this interchange needs its
// own type rather than aliases of the generic carrier.
using opcode_index = precept::index_below<8>;
using register_index = precept::index_below<8>;
static_assert(std::same_as<opcode_index, register_index>);

} // namespace
