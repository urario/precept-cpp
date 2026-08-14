// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>

namespace precept {

namespace detail {

template <class T>
concept complete_object_type = std::is_object_v<T> && requires { sizeof(T); };

} // namespace detail

/// A nullable, non-owning pointer whose non-null current address is aligned to at least `N` bytes.
///
/// The pointer does not own or extend the lifetime of the pointed-to object. The guarantee applies
/// only to the stored address; it does not describe how the storage obtained its alignment or
/// preserve alignment after pointer arithmetic. Use `try_from()` to validate a raw pointer.
template <class T, std::size_t N>
  requires(detail::complete_object_type<T> && N > 0 && (N & (N - 1)) == 0 && N >= alignof(T))
class aligned_ptr {
public:
  static constexpr std::size_t alignment = N;

  /// Implicitly preserves compatible qualifications while weakening the alignment guarantee.
  template <class U, std::size_t M>
    requires(M >= N && std::is_convertible_v<U (*)[], T (*)[]>)
  constexpr aligned_ptr(const aligned_ptr<U, M>& source) noexcept : pointer_(source.get()) {}

  /// Validates the current address of `pointer` without taking ownership.
  ///
  /// Returns `std::nullopt` for a non-null pointer that is not aligned to `N` bytes. A null pointer
  /// is a valid pointer state and produces an engaged optional whose `get()` is null. This function
  /// does not provide a non-throwing guarantee.
  ///
  /// \pre `pointer` is null, or it points to a live `T` object backed by at least `sizeof(T)` bytes
  /// of contiguous storage.
  [[nodiscard]] static std::optional<aligned_ptr> try_from(T* pointer) {
    if (pointer == nullptr) {
      return aligned_ptr(validated_t{}, pointer);
    }

    void* candidate = const_cast<void*>(static_cast<const volatile void*>(pointer));
    const void* original = candidate;
    std::size_t space = sizeof(T);
    if (std::align(N, sizeof(T), candidate, space) != original) {
      return std::nullopt;
    }

    return aligned_ptr(validated_t{}, pointer);
  }

  /// Returns the stored pointer as a named weakening of the alignment guarantee.
  ///
  /// No optimizer assumption is applied. A downstream standard facility such as
  /// `std::assume_aligned` retains its own preconditions.
  [[nodiscard]] constexpr T* get() const noexcept { return pointer_; }

private:
  struct validated_t {};

  constexpr aligned_ptr(validated_t, T* pointer) noexcept : pointer_(pointer) {}

  T* pointer_;
};

} // namespace precept
