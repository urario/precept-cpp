// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>

namespace precept {

namespace detail {

template <class From, class To>
concept qualification_compatible = std::is_convertible_v<From (*)[], To (*)[]>;

} // namespace detail

/// A non-owning contiguous view that contains at least `N` elements.
///
/// The view does not own or extend the lifetime of its elements. Runtime-sized sources must be
/// validated with `try_from()`; a size mismatch is reported as `std::nullopt`.
///
/// This type models `std::ranges::contiguous_range`, `std::ranges::sized_range`, and
/// `std::ranges::borrowed_range`. A compatible dynamic-extent `std::span` may therefore be
/// constructed implicitly as a safe weakening of the minimum-size guarantee.
template <class T, std::size_t N>
  requires(N > 0 && N != std::dynamic_extent)
class at_least_span {
public:
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;
  using iterator = typename std::span<T>::iterator;

  static constexpr size_type minimum_size = N;

  at_least_span() = delete;
  constexpr at_least_span(const at_least_span&) noexcept = default;
  constexpr at_least_span(at_least_span&&) noexcept = default;
  constexpr at_least_span& operator=(const at_least_span&) noexcept = default;
  constexpr at_least_span& operator=(at_least_span&&) noexcept = default;

  /// Implicitly constructs from a fixed-extent `std::span` that proves `size() >= N`.
  template <class U, std::size_t E>
    requires(E != std::dynamic_extent && E >= N && detail::qualification_compatible<U, T>)
  constexpr at_least_span(std::span<U, E> source) noexcept : view_(source) {}

  /// Implicitly weakens an equal or stronger minimum-size guarantee.
  template <class U, std::size_t M>
    requires(M >= N && detail::qualification_compatible<U, T>)
  constexpr at_least_span(const at_least_span<U, M>& source) noexcept : view_(source.as_span()) {}

  /// Validates a dynamic-extent `std::span` without throwing or truncating it.
  ///
  /// Returns an engaged optional exactly when `source.size() >= N`. The returned view is non-owning
  /// and follows the same lifetime and invalidation rules as `source`.
  template <class U, std::size_t E>
    requires(E == std::dynamic_extent && detail::qualification_compatible<U, T>)
  [[nodiscard]] static constexpr std::optional<at_least_span>
  try_from(std::span<U, E> source) noexcept {
    if (source.size() < N) {
      return std::nullopt;
    }
    return at_least_span(validated_t{}, std::span<T>{source});
  }

  /// Returns a dynamic-extent `std::span` over the complete view.
  [[nodiscard]] constexpr std::span<T> as_span() const noexcept { return view_; }

  /// Returns the guaranteed first `N` elements as a fixed-extent `std::span`.
  [[nodiscard]] constexpr std::span<T, N> prefix() const noexcept {
    return view_.template first<N>();
  }

  /// Returns the elements following the guaranteed prefix.
  [[nodiscard]] constexpr std::span<T> rest() const noexcept { return view_.subspan(N); }

  [[nodiscard]] constexpr size_type size() const noexcept { return view_.size(); }
  [[nodiscard]] constexpr size_type size_bytes() const noexcept { return view_.size_bytes(); }
  [[nodiscard]] constexpr pointer data() const noexcept { return view_.data(); }

  /// Returns the first element, which always exists.
  [[nodiscard]] constexpr reference front() const noexcept { return view_.front(); }

  /// Returns the last element, which always exists.
  [[nodiscard]] constexpr reference back() const noexcept { return view_.back(); }

  /// Returns the element at `index`.
  ///
  /// \pre `index < size()`.
  [[nodiscard]] constexpr reference operator[](size_type index) const noexcept {
    return view_[index];
  }
  [[nodiscard]] constexpr iterator begin() const noexcept { return view_.begin(); }
  [[nodiscard]] constexpr iterator end() const noexcept { return view_.end(); }

private:
  struct validated_t {};

  constexpr at_least_span(validated_t, std::span<T> source) noexcept : view_(source) {}

  std::span<T> view_;
};

/// An alias for `at_least_span<T, 1>` that guarantees at least one element.
template <class T> using non_empty_span = at_least_span<T, 1>;

} // namespace precept

namespace std::ranges {

template <class T, std::size_t N>
  requires(N > 0 && N != std::dynamic_extent)
inline constexpr bool enable_borrowed_range<precept::at_least_span<T, N>> = true;

} // namespace std::ranges
