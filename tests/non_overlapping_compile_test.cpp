// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/non_overlapping.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>

namespace {

template <class T, class U>
concept can_check_non_overlap = requires(std::span<T> first, std::span<U> second) {
  precept::checked_non_overlapping(first, second);
};

using mutable_pair = precept::non_overlapping_spans<std::byte, std::byte>;
using qualified_pair = precept::non_overlapping_spans<std::byte, const std::byte>;
using heterogeneous_pair = precept::non_overlapping_spans<char, const unsigned char>;

static_assert(!std::default_initializable<mutable_pair>);
static_assert(!std::constructible_from<mutable_pair, std::span<std::byte>, std::span<std::byte>>);
static_assert(std::copy_constructible<mutable_pair>);
static_assert(std::movable<mutable_pair>);
static_assert(std::is_trivially_copyable_v<mutable_pair>);

static_assert(std::same_as<mutable_pair::first_element_type, std::byte>);
static_assert(std::same_as<qualified_pair::second_element_type, const std::byte>);
static_assert(std::same_as<heterogeneous_pair::first_element_type, char>);
static_assert(
    std::same_as<decltype(std::declval<const mutable_pair&>().first()), std::span<std::byte>>);
static_assert(std::same_as<decltype(std::declval<const qualified_pair&>().second()),
                           std::span<const std::byte>>);
static_assert(noexcept(std::declval<const mutable_pair&>().first()));
static_assert(noexcept(std::declval<const mutable_pair&>().second()));

static_assert(can_check_non_overlap<std::byte, std::byte>);
static_assert(can_check_non_overlap<std::byte, const std::byte>);
static_assert(can_check_non_overlap<char, const unsigned char>);
static_assert(can_check_non_overlap<volatile std::byte, const volatile std::byte>);
static_assert(!can_check_non_overlap<int, int>);
static_assert(!can_check_non_overlap<std::byte, int>);
static_assert(!can_check_non_overlap<std::uint16_t, std::byte>);

static_assert(
    std::same_as<decltype(precept::checked_non_overlapping(
                     std::declval<std::span<std::byte, 2>>(),
                     std::declval<std::span<const std::byte, 3>>())),
                 std::optional<precept::non_overlapping_spans<std::byte, const std::byte>>>);
static_assert(noexcept(precept::checked_non_overlapping(
    std::declval<std::span<std::byte>>(), std::declval<std::span<const std::byte>>())));

} // namespace
