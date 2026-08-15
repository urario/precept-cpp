// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/never_decrease.hpp>

#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace {

template <class T>
concept valid_never_decrease = requires { typename precept::never_decrease<T>; };

template <class T>
concept has_addition = requires(const T& value) { value + 1; };

using int_value = precept::never_decrease<int>;
using unsigned_value = precept::never_decrease<std::uint64_t>;

static_assert(valid_never_decrease<int>);
static_assert(valid_never_decrease<unsigned int>);
static_assert(valid_never_decrease<char>);
static_assert(!valid_never_decrease<bool>);
static_assert(!valid_never_decrease<const int>);
static_assert(!valid_never_decrease<volatile int>);
static_assert(!valid_never_decrease<float>);
static_assert(!valid_never_decrease<double>);
static_assert(!valid_never_decrease<void>);
static_assert(!valid_never_decrease<int&>);

static_assert(!std::default_initializable<int_value>);
static_assert(std::constructible_from<int_value, int>);
static_assert(!std::convertible_to<int, int_value>);
static_assert(!std::convertible_to<int_value, int>);
static_assert(std::copy_constructible<int_value>);
static_assert(std::move_constructible<int_value>);
static_assert(std::is_trivially_copyable_v<int_value>);
static_assert(!std::assignable_from<int_value&, const int_value&>);
static_assert(!std::assignable_from<int_value&, int_value&&>);

static_assert(std::same_as<decltype(std::declval<const int_value&>().value()), int>);
static_assert(std::same_as<decltype(std::declval<int_value&>().value()), int>);
static_assert(std::same_as<decltype(std::declval<int_value&>().try_update(0)), bool>);
static_assert(noexcept(int_value{0}));
static_assert(noexcept(std::declval<const int_value&>().value()));
static_assert(noexcept(std::declval<int_value&>().try_update(0)));

static_assert(!has_addition<int_value>);

constexpr bool supports_constant_evaluation() {
  precept::never_decrease<int> value{-2};
  return value.value() == -2 && value.try_update(-2) && value.try_update(3) &&
         !value.try_update(2) && value.value() == 3;
}

static_assert(supports_constant_evaluation());

constexpr unsigned_value maximum_value{std::numeric_limits<std::uint64_t>::max()};
static_assert(maximum_value.value() == std::numeric_limits<std::uint64_t>::max());

} // namespace
