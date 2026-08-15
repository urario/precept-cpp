// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <type_traits>

namespace precept {

namespace detail {

template <class T>
concept byte_viewable_span_element = !std::is_volatile_v<T>;

inline bool contains_byte_address(std::span<const std::byte> bytes,
                                  const std::byte* address) noexcept {
  for (const std::byte& byte : bytes) {
    if (&byte == address) {
      return true;
    }
  }
  return false;
}

template <class T, std::size_t E, class U, std::size_t F>
  requires byte_viewable_span_element<T> && byte_viewable_span_element<U>
[[nodiscard]] bool storage_ranges_overlap(std::span<T, E> first, std::span<U, F> second) noexcept {
  if (first.empty() || second.empty()) {
    return false;
  }

  const std::span<const std::byte> first_bytes = std::as_bytes(first);
  const std::span<const std::byte> second_bytes = std::as_bytes(second);

  // For two non-empty contiguous intervals, they overlap exactly when the first byte of either
  // interval is contained in the other. Equality is sufficient; unrelated pointers are never
  // ordered or converted to integers.
  return contains_byte_address(first_bytes, second_bytes.data()) ||
         contains_byte_address(second_bytes, first_bytes.data());
}

} // namespace detail

template <class T, class U>
  requires detail::byte_viewable_span_element<T> && detail::byte_viewable_span_element<U>
class non_overlapping_spans;

template <class T, std::size_t E, class U, std::size_t F>
  requires detail::byte_viewable_span_element<T> && detail::byte_viewable_span_element<U>
[[nodiscard]] std::optional<non_overlapping_spans<T, U>>
checked_non_overlapping(std::span<T, E> first, std::span<U, F> second) noexcept;

/// Two non-owning contiguous views whose object-representation byte ranges do not overlap.
///
/// An empty view overlaps nothing, and non-empty views that only touch at an endpoint do not
/// overlap. The stored spans are snapshots: assigning a different range to either source span
/// after validation does not change this object, and changing element values cannot change the
/// relation.
///
/// The object does not own or extend the lifetime of either range. Destruction, reallocation, or
/// storage reuse that invalidates a stored `std::span` also invalidates use of this object.
/// Volatile element types are not accepted because C++20 `std::as_bytes` does not preserve
/// volatile qualification.
template <class T, class U>
  requires detail::byte_viewable_span_element<T> && detail::byte_viewable_span_element<U>
class non_overlapping_spans {
public:
  using first_element_type = T;
  using second_element_type = U;

  constexpr non_overlapping_spans(const non_overlapping_spans&) noexcept = default;
  constexpr non_overlapping_spans(non_overlapping_spans&&) noexcept = default;
  constexpr non_overlapping_spans& operator=(const non_overlapping_spans&) noexcept = default;
  constexpr non_overlapping_spans& operator=(non_overlapping_spans&&) noexcept = default;

  /// Returns the first stored span by value.
  [[nodiscard]] constexpr std::span<T> first() const noexcept { return first_; }

  /// Returns the second stored span by value.
  [[nodiscard]] constexpr std::span<U> second() const noexcept { return second_; }

private:
  struct validated_t {};

  constexpr non_overlapping_spans(validated_t, std::span<T> first, std::span<U> second) noexcept
      : first_(first), second_(second) {}

  template <class First, std::size_t FirstExtent, class Second, std::size_t SecondExtent>
    requires detail::byte_viewable_span_element<First> && detail::byte_viewable_span_element<Second>
  friend std::optional<non_overlapping_spans<First, Second>>
  checked_non_overlapping(std::span<First, FirstExtent> first,
                          std::span<Second, SecondExtent> second) noexcept;

  std::span<T> first_;
  std::span<U> second_;
};

/// Validates that two spans share no byte of object representation.
///
/// Returns a pair of stored span snapshots when their byte ranges are disjoint, including when
/// either range is empty or when the ranges only touch at an endpoint. Returns `std::nullopt` when
/// at least one byte is shared. Validation is linear in the sum of both byte extents, does not read
/// element values, and does not order unrelated pointers. Volatile element types are not accepted.
template <class T, std::size_t E, class U, std::size_t F>
  requires detail::byte_viewable_span_element<T> && detail::byte_viewable_span_element<U>
[[nodiscard]] std::optional<non_overlapping_spans<T, U>>
checked_non_overlapping(std::span<T, E> first, std::span<U, F> second) noexcept {
  if (detail::storage_ranges_overlap(first, second)) {
    return std::nullopt;
  }

  using result_type = non_overlapping_spans<T, U>;
  return result_type(typename result_type::validated_t{}, std::span<T>{first},
                     std::span<U>{second});
}

} // namespace precept
