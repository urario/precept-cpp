// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <optional>

namespace precept {

/// An index verified to be less than the compile-time bound `N`.
///
/// The type carries exactly one fact: `value() < N`. It does not identify a particular table,
/// field, container, or other domain, and it does not provide arithmetic or implicit conversion to
/// `std::size_t`. Use `try_from()` to validate an index and `value()` when indexing a fixed-extent
/// standard-library type.
///
/// `index_below<0>` is a valid type with no publicly constructible values. Its `try_from()` always
/// returns `std::nullopt`.
template <std::size_t N> class index_below {
public:
  /// Validates that `index` is less than `N`.
  ///
  /// Returns `std::nullopt` when `index >= N`. Otherwise the result stores `index` unchanged.
  [[nodiscard]] static constexpr std::optional<index_below> try_from(std::size_t index) noexcept {
    if constexpr (N == 0) {
      return std::nullopt;
    } else {
      if (index >= N) {
        return std::nullopt;
      }

      return index_below(validated_t{}, index);
    }
  }

  /// Implicitly widens a validated upper-bound guarantee.
  ///
  /// When `M <= N`, the source fact `value() < M` already satisfies `value() < N`. The validated
  /// scalar is preserved without runtime validation. A conversion to a smaller bound is not
  /// provided; use `try_from()` to establish that stronger guarantee.
  template <std::size_t M>
    requires(M <= N)
  constexpr index_below(const index_below<M>& source) noexcept : index_(source.value()) {}

  /// Returns the validated index as an ordinary `std::size_t`.
  [[nodiscard]] constexpr std::size_t value() const noexcept { return index_; }

private:
  struct validated_t {};

  constexpr index_below(validated_t, std::size_t index) noexcept : index_(index) {}

  std::size_t index_;
};

} // namespace precept
