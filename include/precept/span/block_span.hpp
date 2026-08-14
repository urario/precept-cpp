// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <compare>
#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>

namespace precept {

/// A non-owning range of fixed-size blocks over contiguous elements.
///
/// Every object contains a whole number of `N`-element blocks. Runtime-sized sources must be
/// validated with `try_from()`; a remainder is reported as `std::nullopt`. The view does not own
/// or extend the lifetime of its elements.
///
/// This type models a common, sized, random-access, and borrowed range whose logical elements are
/// `std::span<T, N>` block views. It is not a contiguous range because adjacent logical elements
/// are spans rather than adjacent `std::span` objects.
template <class T, std::size_t N>
  requires(N > 0 && N != std::dynamic_extent)
class block_span {
public:
  using element_type = T;
  using block_type = std::span<T, N>;
  using value_type = block_type;
  using reference = block_type;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  class iterator {
  public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::input_iterator_tag;
    using value_type = block_type;
    using difference_type = std::ptrdiff_t;
    using reference = block_type;

    constexpr iterator() noexcept = default;

    [[nodiscard]] constexpr reference operator*() const noexcept {
      return reference{base_ + index_ * static_cast<difference_type>(N), N};
    }

    [[nodiscard]] constexpr reference operator[](difference_type offset) const noexcept {
      return *(*this + offset);
    }

    constexpr iterator& operator++() noexcept {
      ++index_;
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      iterator previous = *this;
      ++*this;
      return previous;
    }

    constexpr iterator& operator--() noexcept {
      --index_;
      return *this;
    }

    constexpr iterator operator--(int) noexcept {
      iterator previous = *this;
      --*this;
      return previous;
    }

    constexpr iterator& operator+=(difference_type offset) noexcept {
      index_ += offset;
      return *this;
    }

    constexpr iterator& operator-=(difference_type offset) noexcept {
      index_ -= offset;
      return *this;
    }

    friend constexpr iterator operator+(iterator position, difference_type offset) noexcept {
      position += offset;
      return position;
    }

    friend constexpr iterator operator+(difference_type offset, iterator position) noexcept {
      position += offset;
      return position;
    }

    friend constexpr iterator operator-(iterator position, difference_type offset) noexcept {
      position -= offset;
      return position;
    }

    friend constexpr difference_type operator-(const iterator& left,
                                               const iterator& right) noexcept {
      return left.index_ - right.index_;
    }

    friend constexpr bool operator==(const iterator&, const iterator&) noexcept = default;
    friend constexpr auto operator<=>(const iterator&, const iterator&) noexcept = default;

  private:
    friend class block_span;

    constexpr iterator(T* base, difference_type index) noexcept : base_(base), index_(index) {}

    T* base_ = nullptr;
    difference_type index_ = 0;
  };

  static constexpr size_type block_size = N;

  constexpr block_span() noexcept = default;
  constexpr block_span(const block_span&) noexcept = default;
  constexpr block_span(block_span&&) noexcept = default;
  constexpr block_span& operator=(const block_span&) noexcept = default;
  constexpr block_span& operator=(block_span&&) noexcept = default;

  /// Implicitly constructs from a fixed-extent span that proves there is no partial block.
  template <class U, std::size_t E>
    requires(E != std::dynamic_extent && E % N == 0 && std::is_convertible_v<U (*)[], T (*)[]>)
  constexpr block_span(std::span<U, E> source) noexcept : view_(source) {}

  /// Implicitly preserves the block partition while adding compatible element qualifications.
  template <class U>
    requires std::is_convertible_v<U (*)[], T (*)[]>
  constexpr block_span(const block_span<U, N>& source) noexcept : view_(source.as_span()) {}

  /// Validates that a dynamic-extent span contains only complete `N`-element blocks.
  ///
  /// Returns an engaged optional exactly when `source.size() % N == 0`. Empty input is valid. The
  /// returned view follows the same lifetime and invalidation rules as `source`.
  template <class U, std::size_t E>
    requires(E == std::dynamic_extent && std::is_convertible_v<U (*)[], T (*)[]>)
  [[nodiscard]] static constexpr std::optional<block_span>
  try_from(std::span<U, E> source) noexcept {
    if (source.size() % N != 0) {
      return std::nullopt;
    }
    return block_span(validated_t{}, std::span<T>{source});
  }

  /// Returns a dynamic-extent span over all underlying elements.
  [[nodiscard]] constexpr std::span<T> as_span() const noexcept { return view_; }

  /// Returns the number of complete blocks in the view.
  [[nodiscard]] constexpr size_type size() const noexcept { return view_.size() / N; }

  /// Returns the number of complete blocks in the view.
  [[nodiscard]] constexpr size_type block_count() const noexcept { return size(); }

  [[nodiscard]] constexpr bool empty() const noexcept { return view_.empty(); }

  /// Returns the block at `index`.
  ///
  /// \pre `index < size()`.
  [[nodiscard]] constexpr block_type operator[](size_type index) const noexcept {
    return block_type{view_.data() + index * N, N};
  }

  [[nodiscard]] constexpr iterator begin() const noexcept { return iterator{view_.data(), 0}; }

  [[nodiscard]] constexpr iterator end() const noexcept {
    return iterator{view_.data(), static_cast<difference_type>(size())};
  }

private:
  struct validated_t {};

  constexpr block_span(validated_t, std::span<T> source) noexcept : view_(source) {}

  std::span<T> view_;
};

} // namespace precept

namespace std::ranges {

template <class T, std::size_t N>
  requires(N > 0 && N != std::dynamic_extent)
inline constexpr bool enable_borrowed_range<precept::block_span<T, N>> = true;

} // namespace std::ranges
