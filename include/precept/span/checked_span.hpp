// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <optional>
#include <span>

namespace precept {

/// Validates that `source` contains exactly `N` elements.
///
/// Returns a fixed-extent view over the same elements when the size matches, or `std::nullopt`
/// otherwise. The function does not truncate the source, own its elements, or extend their
/// lifetime.
///
/// `N` may be zero. A fixed-extent source is accepted only when its extent equals `N`, and
/// `std::dynamic_extent` is not a valid exact size.
template <std::size_t N, class T, std::size_t E>
  requires(N != std::dynamic_extent && (E == std::dynamic_extent || E == N))
[[nodiscard]] constexpr std::optional<std::span<T, N>>
checked_span(std::span<T, E> source) noexcept {
  if (source.size() != N) {
    return std::nullopt;
  }
  return source.template first<N>();
}

} // namespace precept
