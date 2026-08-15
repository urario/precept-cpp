// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/set_once.hpp>

#include <concepts>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace {

struct move_only {
  move_only() = default;
  move_only(const move_only&) = delete;
  move_only(move_only&&) = default;
};

struct immobile {
  immobile() = default;
  immobile(const immobile&) = delete;
  immobile(immobile&&) = delete;
};

struct throwing_move {
  throwing_move() = default;
  throwing_move(const throwing_move&) = delete;
  throwing_move(throwing_move&&) noexcept(false) {}
};

struct non_default {
  explicit non_default(int) {}
};

struct incomplete;

template <class T>
concept valid_set_once = requires { typename precept::set_once<T>; };

template <class T>
concept has_dereference = requires(T& value) { *value; };

template <class T>
concept has_arrow = requires(T& value) { value.operator->(); };

template <class T>
concept observes_rvalue = requires(T value) { std::move(value).value(); };

using int_slot = precept::set_once<int>;
using string_slot = precept::set_once<std::string>;

static_assert(valid_set_once<int>);
static_assert(valid_set_once<const int>);
static_assert(valid_set_once<std::nullopt_t>);
static_assert(valid_set_once<std::in_place_t>);
static_assert(!valid_set_once<void>);
static_assert(!valid_set_once<int&>);
static_assert(!valid_set_once<int[2]>);
static_assert(!valid_set_once<incomplete>);

static_assert(std::default_initializable<int_slot>);
static_assert(std::default_initializable<precept::set_once<non_default>>);
static_assert(std::copy_constructible<int_slot>);
static_assert(std::move_constructible<int_slot>);
static_assert(std::is_nothrow_move_constructible_v<int_slot>);
static_assert(!std::assignable_from<int_slot&, const int_slot&>);
static_assert(!std::assignable_from<int_slot&, int_slot&&>);

static_assert(!std::copy_constructible<precept::set_once<move_only>>);
static_assert(std::move_constructible<precept::set_once<move_only>>);
static_assert(!std::copy_constructible<precept::set_once<immobile>>);
static_assert(!std::move_constructible<precept::set_once<immobile>>);
static_assert(std::move_constructible<precept::set_once<throwing_move>>);
static_assert(!std::is_nothrow_move_constructible_v<precept::set_once<throwing_move>>);

static_assert(std::same_as<decltype(std::declval<const int_slot&>().value()), const int&>);
static_assert(std::same_as<decltype(std::declval<int_slot&>().value()), const int&>);
static_assert(
    std::same_as<decltype(std::declval<int_slot&>().try_set(std::declval<const int&>())), bool>);
static_assert(
    std::same_as<decltype(std::declval<int_slot&>().try_set(std::declval<int&&>())), bool>);
static_assert(std::same_as<decltype(std::declval<string_slot&>().try_emplace(3, 'x')), bool>);

static_assert(noexcept(std::declval<const int_slot&>().has_value()));
static_assert(!noexcept(std::declval<const int_slot&>().value()));
static_assert(!noexcept(std::declval<int_slot&>().try_set(std::declval<int&&>())));

static_assert(!std::convertible_to<int_slot, bool>);
static_assert(!has_dereference<int_slot>);
static_assert(!has_arrow<int_slot>);
static_assert(!observes_rvalue<int_slot>);

constexpr bool supports_constant_evaluation() {
  precept::set_once<int> value;
  if (value.has_value() || !value.try_emplace(42) || !value.has_value()) {
    return false;
  }
  return value.value() == 42 && !value.try_set(7);
}

static_assert(supports_constant_evaluation());

} // namespace
