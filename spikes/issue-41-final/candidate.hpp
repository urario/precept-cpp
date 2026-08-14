#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>

namespace spike {

namespace detail {

template <class T>
concept complete_object_type = std::is_object_v<T> && requires { sizeof(T); };

template <class From, class To>
concept qualification_compatible = std::is_convertible_v<From (*)[], To (*)[]>;

} // namespace detail

template <class T, std::size_t N>
  requires detail::complete_object_type<T> && (N != 0) && ((N & (N - 1)) == 0) &&
           (N >= alignof(T))
class aligned_ptr {
  struct validated_t {};
  T* ptr_{};

  constexpr aligned_ptr(T* ptr, validated_t) noexcept : ptr_(ptr) {}

public:
  aligned_ptr() = delete;
  constexpr aligned_ptr(const aligned_ptr&) noexcept = default;
  constexpr aligned_ptr(aligned_ptr&&) noexcept = default;
  constexpr aligned_ptr& operator=(const aligned_ptr&) noexcept = default;
  constexpr aligned_ptr& operator=(aligned_ptr&&) noexcept = default;

  [[nodiscard]] static std::optional<aligned_ptr> try_from(T* ptr) {
    if (ptr == nullptr) {
      return aligned_ptr{nullptr, validated_t{}};
    }

    const void* const original = static_cast<const void*>(ptr);
    void* candidate = const_cast<void*>(original);
    std::size_t space = sizeof(T);

    void* const result = std::align(N, sizeof(T), candidate, space);
    if (result != original) {
      return std::nullopt;
    }

    return aligned_ptr{ptr, validated_t{}};
  }

  template <class U, std::size_t M>
    requires(M >= N && detail::qualification_compatible<U, T>)
  constexpr aligned_ptr(const aligned_ptr<U, M>& other) noexcept : ptr_(other.get()) {}

  [[nodiscard]] constexpr T* get() const noexcept { return ptr_; }

  static constexpr std::size_t alignment = N;
};

} // namespace spike
