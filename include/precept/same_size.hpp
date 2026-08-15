// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <optional>
#include <span>

namespace precept {

template <class T, class U> class same_size_pair;

template <class T, class U, std::size_t E, std::size_t F>
  requires(E == std::dynamic_extent && F == std::dynamic_extent)
[[nodiscard]] constexpr std::optional<same_size_pair<T, U>>
checked_same_size(std::span<T, E> first, std::span<U, F> second) noexcept;

/// A pair of non-owning dynamic spans with equal element counts.
///
/// A valid object guarantees only that `first().size() == second().size()`. It does not imply
/// element correspondence, a shared domain or owner, a shared lifetime, non-overlap, alignment,
/// sorting, or uniqueness. The stored spans are snapshots; assigning a different span to either
/// source variable after validation does not retarget this object.
///
/// The object does not own or extend the lifetime of either range. Destruction, reallocation, or
/// storage reuse that invalidates a stored `std::span` also invalidates use of this object.
template <class T, class U> class same_size_pair {
public:
  constexpr same_size_pair(const same_size_pair&) noexcept = default;
  constexpr same_size_pair(same_size_pair&&) noexcept = default;
  constexpr same_size_pair& operator=(const same_size_pair&) noexcept = default;
  constexpr same_size_pair& operator=(same_size_pair&&) noexcept = default;

  /// Returns the first stored span by value.
  [[nodiscard]] constexpr std::span<T> first() const noexcept { return first_; }

  /// Returns the second stored span by value.
  [[nodiscard]] constexpr std::span<U> second() const noexcept { return second_; }

  /// Returns the common element count of the stored spans.
  [[nodiscard]] constexpr std::size_t size() const noexcept { return first_.size(); }

private:
  struct validated_t {};

  constexpr same_size_pair(validated_t, std::span<T> first, std::span<U> second) noexcept
      : first_(first), second_(second) {}

  template <class First, class Second, std::size_t FirstExtent, std::size_t SecondExtent>
    requires(FirstExtent == std::dynamic_extent && SecondExtent == std::dynamic_extent)
  friend constexpr std::optional<same_size_pair<First, Second>>
  checked_same_size(std::span<First, FirstExtent> first,
                    std::span<Second, SecondExtent> second) noexcept;

  std::span<T> first_;
  std::span<U> second_;
};

/// Validates that two dynamic-extent spans have the same number of elements.
///
/// Returns a carrier containing span snapshots when `first.size() == second.size()`, including
/// for two empty spans. Returns `std::nullopt` for unequal sizes. The function does not inspect or
/// modify element values, and the carrier does not claim semantic correspondence between elements.
/// Fixed-extent spans are intentionally not accepted by this factory; use the standard extent in
/// `std::span<T, N>` when the equality is already represented by the type.
template <class T, class U, std::size_t E, std::size_t F>
  requires(E == std::dynamic_extent && F == std::dynamic_extent)
[[nodiscard]] constexpr std::optional<same_size_pair<T, U>>
checked_same_size(std::span<T, E> first, std::span<U, F> second) noexcept {
  if (first.size() != second.size()) {
    return std::nullopt;
  }

  using result_type = same_size_pair<T, U>;
  return result_type(typename result_type::validated_t{}, first, second);
}

} // namespace precept
