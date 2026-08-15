// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <span>

namespace precept {

namespace detail {

template <class T>
concept byte_sized_span_element = sizeof(T) == 1;

template <class T, std::size_t E, class U>
[[nodiscard]] bool contains_element_address(std::span<T, E> elements, U* address) noexcept {
  const volatile void* const target = static_cast<const volatile void*>(address);
  for (T& element : elements) {
    if (static_cast<const volatile void*>(std::addressof(element)) == target) {
      return true;
    }
  }
  return false;
}

template <class T, std::size_t E, class U, std::size_t F>
  requires byte_sized_span_element<T> && byte_sized_span_element<U>
[[nodiscard]] bool storage_ranges_overlap(std::span<T, E> first, std::span<U, F> second) noexcept {
  if (first.empty() || second.empty()) {
    return false;
  }

  // Each element occupies exactly one byte, so the spans' storage ranges are the contiguous
  // sequences of their actual element objects. Two non-empty ranges overlap exactly when the
  // first element address of either range occurs among the other range's element addresses.
  // This uses pointer equality only; it does not create an object-representation byte view,
  // order unrelated pointers, or convert pointers to integers.
  return contains_element_address(first, second.data()) ||
         contains_element_address(second, first.data());
}

} // namespace detail

template <class T, class U>
  requires detail::byte_sized_span_element<T> && detail::byte_sized_span_element<U>
class non_overlapping_spans;

template <class T, std::size_t E, class U, std::size_t F>
  requires detail::byte_sized_span_element<T> && detail::byte_sized_span_element<U>
[[nodiscard]] std::optional<non_overlapping_spans<T, U>>
checked_non_overlapping(std::span<T, E> first, std::span<U, F> second) noexcept;

/// Two non-owning contiguous views of byte-sized elements whose storage ranges share no byte.
///
/// Each element type must occupy exactly one byte. An empty view overlaps nothing, and non-empty
/// views that only touch at an endpoint do not overlap. The stored spans are snapshots: assigning
/// a different range to either source span after validation does not change this object, and
/// changing element values cannot change the relation.
///
/// The object does not own or extend the lifetime of either range. Destruction, reallocation, or
/// storage reuse that invalidates a stored `std::span` also invalidates use of this object.
template <class T, class U>
  requires detail::byte_sized_span_element<T> && detail::byte_sized_span_element<U>
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
    requires detail::byte_sized_span_element<First> && detail::byte_sized_span_element<Second>
  friend std::optional<non_overlapping_spans<First, Second>>
  checked_non_overlapping(std::span<First, FirstExtent> first,
                          std::span<Second, SecondExtent> second) noexcept;

  std::span<T> first_;
  std::span<U> second_;
};

/// Validates that two spans of byte-sized elements share no byte of storage.
///
/// Returns a pair of stored span snapshots when their storage ranges are disjoint, including when
/// either range is empty or when the ranges only touch at an endpoint. Returns `std::nullopt` when
/// at least one byte is shared. Validation is linear in the sum of both element counts, does not
/// read element values, and does not order unrelated pointers or convert them to integers.
template <class T, std::size_t E, class U, std::size_t F>
  requires detail::byte_sized_span_element<T> && detail::byte_sized_span_element<U>
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
