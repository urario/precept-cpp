// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <concepts>
#include <type_traits>

namespace precept {

namespace detail {

template <class T>
concept never_decrease_value =
    std::integral<T> && !std::same_as<T, bool> && std::same_as<T, std::remove_cv_t<T>>;

} // namespace detail

/// A value whose history can only stay the same or move upward through `try_update`.
///
/// Each object owns its own history: construction records the initial value, an update succeeds
/// when `next >= value()`, and a rejected regression leaves the stored value unchanged. The type
/// is an ordinary non-thread-safe value type; it does not provide synchronization, shared history,
/// persistence, or arithmetic operations.
template <class T>
  requires detail::never_decrease_value<T>
class never_decrease {
public:
  /// Constructs a value with its initial history entry.
  explicit constexpr never_decrease(T initial) noexcept : value_(initial) {}

  constexpr never_decrease(const never_decrease&) = default;
  constexpr never_decrease(never_decrease&&) = default;

  never_decrease& operator=(const never_decrease&) = delete;
  never_decrease& operator=(never_decrease&&) = delete;

  /// Returns the current value in this object's non-decreasing history.
  [[nodiscard]] constexpr T value() const noexcept { return value_; }

  /// Accepts `next` when it is not below the current value.
  ///
  /// Equal values are accepted. A rejected regression returns `false` and does not change the
  /// stored value. The comparison is the only transition validation performed; arithmetic used by
  /// the caller to produce `next` remains outside this type's contract.
  [[nodiscard]] constexpr bool try_update(T next) noexcept {
    if (next < value_) {
      return false;
    }

    value_ = next;
    return true;
  }

private:
  T value_;
};

} // namespace precept
