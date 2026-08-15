// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/narrow_exact.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

namespace {

enum class scoped_enum : int { zero = 0 };
enum plain_enum { plain_zero = 0 };
struct incomplete;

template <class T, class U>
concept narrowable = requires { precept::narrow_exact<T>(std::declval<U>()); };

// Admitted conversions: signed and unsigned integer types, in either direction, narrowing or
// widening. `std::uint8_t` is `unsigned char`, a plain unsigned integer type, so byte-width
// protocol fields are covered.
static_assert(narrowable<std::uint16_t, std::size_t>);
static_assert(narrowable<std::int32_t, std::uint64_t>);
static_assert(narrowable<std::uint32_t, std::int64_t>);
static_assert(narrowable<std::uint8_t, int>);
static_assert(narrowable<std::int64_t, std::int8_t>);
static_assert(narrowable<long long, unsigned long>);
static_assert(narrowable<int, int>);

// Rejected element types. `bool` and the character types are excluded on both sides: they are the
// types `std::in_range` does not accept, and `char` additionally has implementation-defined
// signedness, so admitting it would make the same conversion platform-dependent.
static_assert(!narrowable<bool, int>);
static_assert(!narrowable<int, bool>);
static_assert(!narrowable<char, int>);
static_assert(!narrowable<int, char>);
static_assert(!narrowable<char16_t, int>);
static_assert(!narrowable<wchar_t, int>);
static_assert(!narrowable<const int, int>);
static_assert(!narrowable<volatile int, int>);
static_assert(!narrowable<double, int>);
static_assert(!narrowable<int, double>);
static_assert(!narrowable<float, float>);
static_assert(!narrowable<void, int>);
static_assert(!narrowable<int, void>);
static_assert(!narrowable<int*, int>);
static_assert(!narrowable<int, int*>);
static_assert(!narrowable<scoped_enum, int>);
static_assert(!narrowable<int, scoped_enum>);
static_assert(!narrowable<int, plain_enum>);
static_assert(!narrowable<int, incomplete>);

// The result is a standard `std::optional<T>`, produced without throwing.
static_assert(std::same_as<decltype(precept::narrow_exact<std::uint16_t>(std::size_t{0})),
                           std::optional<std::uint16_t>>);
static_assert(noexcept(precept::narrow_exact<std::uint16_t>(std::size_t{0})));

constexpr bool supports_constant_evaluation() {
  constexpr auto too_large = std::size_t{std::numeric_limits<std::uint16_t>::max()} + 1U;

  const auto fits = precept::narrow_exact<std::uint16_t>(std::size_t{4096});
  if (!fits || *fits != 4096) {
    return false;
  }

  if (precept::narrow_exact<std::uint16_t>(too_large)) {
    return false;
  }

  return !precept::narrow_exact<std::uint32_t>(std::int64_t{-1});
}

static_assert(supports_constant_evaluation());

// A compile-time constant keeps its narrowed type without a second check at the use site.
inline constexpr std::uint16_t default_frame_size = *precept::narrow_exact<std::uint16_t>(1500);
static_assert(default_frame_size == 1500);

} // namespace
