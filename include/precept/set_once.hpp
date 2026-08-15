// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace precept {

namespace detail {

template <class T>
concept set_once_value =
    std::is_object_v<T> && !std::is_array_v<T> && requires { sizeof(T); } && std::destructible<T>;

} // namespace detail

/// A non-thread-safe value holder whose slot can transition from unset to set at most once.
///
/// The guarantee applies to this object's slot, not to deep immutability of the contained value.
/// A contained object may still expose mutation through its own const interface or through an
/// external alias. Copy and move construction preserve whether the new slot is set. A moved-from
/// `set_once` remains set and contains the moved-from `T`.
template <class T>
  requires detail::set_once_value<T>
class set_once {
  struct stored_value {
    template <class... Args>
      requires std::constructible_from<T, Args...>
    constexpr explicit stored_value(std::in_place_t, Args&&... args)
        : contained(std::forward<Args>(args)...) {}

    T contained;
  };

public:
  constexpr set_once() noexcept = default;

  constexpr set_once(const set_once&) = default;
  constexpr set_once(set_once&&) = default;

  set_once& operator=(const set_once&) = delete;
  set_once& operator=(set_once&&) = delete;

  /// Returns whether this object's slot has been set.
  [[nodiscard]] constexpr bool has_value() const noexcept { return value_.has_value(); }

  /// Returns the contained value without exposing mutable access through this wrapper.
  ///
  /// Throws `std::bad_optional_access` when the slot is unset. This observer is lvalue-only so a
  /// reference cannot be obtained from a temporary `set_once`.
  [[nodiscard]] constexpr const T& value() const& { return value_.value().contained; }
  const T& value() const&& = delete;

  /// Copies `value` into an unset slot.
  ///
  /// Returns false without copying `value` when the slot is already set. If construction throws,
  /// the exception propagates and the slot remains unset.
  [[nodiscard]] constexpr bool try_set(const T& value)
    requires std::constructible_from<T, const T&>
  {
    return try_emplace(value);
  }

  /// Moves `value` into an unset slot.
  ///
  /// Returns false without moving from `value` when the slot is already set. If construction
  /// throws, the exception propagates and the slot remains unset.
  [[nodiscard]] constexpr bool try_set(T&& value)
    requires std::constructible_from<T, T&&>
  {
    return try_emplace(std::move(value));
  }

  /// Constructs `T` directly in an unset slot.
  ///
  /// Returns false without constructing `T` or evaluating the constructor body when the slot is
  /// already set. If construction throws, the exception propagates and the slot remains unset.
  template <class... Args>
    requires std::constructible_from<T, Args...>
  [[nodiscard]] constexpr bool try_emplace(Args&&... args) {
    if (value_.has_value()) {
      return false;
    }

    value_.emplace(std::in_place, std::forward<Args>(args)...);
    return true;
  }

private:
  std::optional<stored_value> value_;
};

} // namespace precept
