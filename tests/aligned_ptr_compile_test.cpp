// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/aligned_ptr.hpp>

#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

using mutable_32 = precept::aligned_ptr<int, 32>;
using const_32 = precept::aligned_ptr<const int, 32>;
using mutable_64 = precept::aligned_ptr<int, 64>;

struct base {};
struct derived : base {};

template <class T, std::size_t N>
concept valid_aligned_ptr = requires { typename precept::aligned_ptr<T, N>; };

static_assert(mutable_32::alignment == 32);
static_assert(sizeof(mutable_32) == sizeof(int*));
static_assert(std::is_trivially_copyable_v<mutable_32>);
static_assert(!std::default_initializable<mutable_32>);
static_assert(std::copyable<mutable_32>);
static_assert(!std::constructible_from<mutable_32, int*>);
static_assert(!std::convertible_to<mutable_32, int*>);

static_assert(std::convertible_to<mutable_64, mutable_32>);
static_assert(std::convertible_to<mutable_64, const_32>);
static_assert(!std::convertible_to<mutable_32, mutable_64>);
static_assert(!std::convertible_to<const_32, mutable_32>);
static_assert(
    !std::convertible_to<precept::aligned_ptr<derived, 64>, precept::aligned_ptr<base, 64>>);

static_assert(
    std::same_as<decltype(mutable_32::try_from(std::declval<int*>())), std::optional<mutable_32>>);
static_assert(std::same_as<decltype(const_32::try_from(std::declval<const int*>())),
                           std::optional<const_32>>);
static_assert(std::same_as<decltype(std::declval<mutable_32>().get()), int*>);
static_assert(std::same_as<decltype(std::declval<const_32>().get()), const int*>);
static_assert(!noexcept(mutable_32::try_from(std::declval<int*>())));
static_assert(noexcept(std::declval<mutable_32>().get()));

static_assert(valid_aligned_ptr<int, alignof(int)>);
static_assert(!valid_aligned_ptr<int, 0>);
static_assert(!valid_aligned_ptr<int, 3>);
static_assert(!valid_aligned_ptr<void, 64>);

} // namespace
