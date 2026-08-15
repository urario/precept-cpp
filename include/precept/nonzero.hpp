// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <concepts>
#include <optional>
#include <type_traits>

namespace precept {

namespace detail {

template <class T>
concept nonzero_value =
    std::integral<T> && !std::same_as<T, bool> && std::same_as<T, std::remove_cv_t<T>>;

} // namespace detail

/// An integral value verified to be different from zero.
///
/// The type carries exactly one fact: the stored value is not zero. It is not a numeric type and
/// not a general numeric wrapper. The guarantee is not propagated through arithmetic, because the
/// operations are not closed over it — the difference of two non-zero values can be zero. Compute
/// on `value()` and validate again when a result has to carry the fact.
///
/// Non-zero is not by itself a sufficient precondition for every use of the value. Signed division
/// additionally requires that the result be representable, so `std::numeric_limits<T>::min() / -1`
/// remains undefined for a validated divisor.
template <class T>
  requires detail::nonzero_value<T>
class nonzero {
public:
  using value_type = T;

  /// Validates that `value` is not zero.
  ///
  /// Returns `std::nullopt` for zero. Otherwise the result stores `value` unchanged, including its
  /// sign.
  [[nodiscard]] static constexpr std::optional<nonzero> try_from(T value) noexcept {
    if (value == T{}) {
      return std::nullopt;
    }

    return nonzero(validated_t{}, value);
  }

  /// Returns the verified value as an ordinary scalar.
  [[nodiscard]] constexpr T value() const noexcept { return value_; }

private:
  struct validated_t {};

  constexpr nonzero(validated_t, T value) noexcept : value_(value) {}

  T value_;
};

} // namespace precept
