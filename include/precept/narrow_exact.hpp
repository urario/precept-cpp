// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace precept {

namespace detail {

/// The integer types `std::in_range` is defined for: signed and unsigned integer types, excluding
/// `bool` and the character types. `char` is left out with them because its signedness is
/// implementation-defined, which would make the same conversion succeed on one platform and fail
/// on another.
template <class T>
concept exact_narrowing_integer =
    std::integral<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool> &&
    !std::same_as<T, char> && !std::same_as<T, wchar_t> && !std::same_as<T, char8_t> &&
    !std::same_as<T, char16_t> && !std::same_as<T, char32_t>;

} // namespace detail

/// Converts `value` to `T` when `T` can represent it exactly.
///
/// Returns the converted value when the integer `value` is representable in `T`, and
/// `std::nullopt` otherwise. Representability is decided on the mathematical value, so a negative
/// value is never converted to an unsigned type and a value above `T`'s maximum is never wrapped.
/// A successful result compares equal to the input.
///
/// The result is an ordinary `T`, not a Precept type: once the value fits, the value itself is the
/// whole of what was verified, and nothing further has to travel with it.
///
/// `T` and `U` are signed or unsigned integer types other than `bool` and the character types —
/// the same set `std::in_range` accepts, which this function uses to make the decision. A widening
/// conversion is admitted and always succeeds.
template <class T, class U>
  requires detail::exact_narrowing_integer<T> && detail::exact_narrowing_integer<U>
[[nodiscard]] constexpr std::optional<T> narrow_exact(U value) noexcept {
  if (!std::in_range<T>(value)) {
    return std::nullopt;
  }

  return static_cast<T>(value);
}

} // namespace precept
