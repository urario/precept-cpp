// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/nonzero.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

enum class scoped_enum : int { none = 0 };
enum plain_enum { plain_zero = 0 };
struct incomplete;

template <class T>
concept valid_nonzero = requires { typename precept::nonzero<T>; };

template <class T>
concept has_addition = requires(const T& left, const T& right) { left + right; };

template <class T>
concept has_subtraction = requires(const T& left, const T& right) { left - right; };

template <class T>
concept has_multiplication = requires(const T& left, const T& right) { left* right; };

template <class T>
concept has_division = requires(const T& left, const T& right) { left / right; };

template <class T>
concept has_negation = requires(const T& value) { -value; };

template <class T>
concept has_equality = requires(const T& left, const T& right) { left == right; };

template <class T>
concept has_ordering = requires(const T& left, const T& right) { left < right; };

template <class T>
concept has_dereference = requires(const T& value) { *value; };

using int_value = precept::nonzero<int>;
using size_value = precept::nonzero<std::size_t>;

// Admitted element types. The fact is only interesting where zero is one of several
// representable values, so `bool` is excluded rather than admitted as a unit type.
static_assert(valid_nonzero<int>);
static_assert(valid_nonzero<unsigned int>);
static_assert(valid_nonzero<long long>);
static_assert(valid_nonzero<std::size_t>);
static_assert(valid_nonzero<std::int8_t>);
static_assert(valid_nonzero<char>);
static_assert(valid_nonzero<char16_t>);
static_assert(!valid_nonzero<bool>);
static_assert(!valid_nonzero<const int>);
static_assert(!valid_nonzero<volatile int>);
static_assert(!valid_nonzero<float>);
static_assert(!valid_nonzero<double>);
static_assert(!valid_nonzero<void>);
static_assert(!valid_nonzero<int&>);
static_assert(!valid_nonzero<int*>);
static_assert(!valid_nonzero<scoped_enum>);
static_assert(!valid_nonzero<plain_enum>);
static_assert(!valid_nonzero<incomplete>);

// The invariant has no bypass: no default state, no raw-value construction, no conversion in.
static_assert(!std::default_initializable<int_value>);
static_assert(!std::constructible_from<int_value, int>);
static_assert(!std::convertible_to<int, int_value>);
static_assert(!std::assignable_from<int_value&, int>);

// A validated value stays validated through copies, and never escapes implicitly.
static_assert(std::copy_constructible<int_value>);
static_assert(std::assignable_from<int_value&, const int_value&>);
static_assert(std::is_trivially_copyable_v<int_value>);
static_assert(sizeof(int_value) == sizeof(int));
static_assert(!std::convertible_to<int_value, int>);
static_assert(!std::convertible_to<int_value, bool>);
static_assert(!std::convertible_to<int_value, size_value>);
static_assert(!has_dereference<int_value>);

// Observation and validation.
static_assert(std::same_as<int_value::value_type, int>);
static_assert(std::same_as<decltype(std::declval<const int_value&>().value()), int>);
static_assert(noexcept(std::declval<const int_value&>().value()));
static_assert(std::same_as<decltype(int_value::try_from(0)), std::optional<int_value>>);
static_assert(noexcept(int_value::try_from(0)));

// The guarantee is not closed over arithmetic, so no arithmetic is offered. Comparison is left out
// for the same reason it is left out of the rest of the vocabulary: it is not part of this fact.
// The `int` row is a control: without it these rejections could hold for a misspelled requirement.
static_assert(has_addition<int> && has_subtraction<int> && has_multiplication<int> &&
              has_division<int> && has_negation<int> && has_equality<int> && has_ordering<int>);
static_assert(!has_addition<int_value>);
static_assert(!has_subtraction<int_value>);
static_assert(!has_multiplication<int_value>);
static_assert(!has_division<int_value>);
static_assert(!has_negation<int_value>);
static_assert(!has_equality<int_value>);
static_assert(!has_ordering<int_value>);

constexpr bool supports_constant_evaluation() {
  const auto negative = int_value::try_from(-7);
  if (!negative || negative->value() != -7) {
    return false;
  }

  return !int_value::try_from(0).has_value();
}

static_assert(supports_constant_evaluation());

// A compile-time constant reaches the type through the same validation as a runtime value.
inline constexpr size_value fixed_stride = *size_value::try_from(8);
static_assert(fixed_stride.value() == 8);

} // namespace
