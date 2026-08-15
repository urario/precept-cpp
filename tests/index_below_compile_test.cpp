// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/index_below.hpp>

#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

template <std::size_t N>
concept valid_index_below = requires { typename precept::index_below<N>; };

using empty_index = precept::index_below<0>;
using single_index = precept::index_below<1>;
using small_index = precept::index_below<8>;
using large_index = precept::index_below<16>;

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
static_assert(sizeof(small_index) == sizeof(std::size_t));
static_assert(!std::convertible_to<small_index, std::size_t>);
static_assert(std::same_as<decltype(std::declval<const small_index&>().value()), std::size_t>);
static_assert(noexcept(std::declval<const small_index&>().value()));
static_assert(std::same_as<decltype(small_index::try_from(0)), std::optional<small_index>>);
static_assert(noexcept(small_index::try_from(0)));

// Safe weakening is deliberately not part of the initial surface. No representative usage needed
// it, and a numeric conversion alone cannot establish that two differently bounded parameters
// belong to the same table or domain.
static_assert(!std::convertible_to<small_index, large_index>);
static_assert(!std::convertible_to<large_index, small_index>);

constexpr bool supports_constant_evaluation() {
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

// Equal bounds do not distinguish domains. A domain that must reject this interchange needs its
// own type rather than aliases of the generic carrier.
using opcode_index = precept::index_below<8>;
using register_index = precept::index_below<8>;
static_assert(std::same_as<opcode_index, register_index>);

} // namespace
