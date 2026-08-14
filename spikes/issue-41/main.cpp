#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>

namespace spike {

template<class T, std::size_t N>
class aligned_ptr {
    static_assert(std::is_object_v<T>);
    static_assert(N != 0 && (N & (N - 1)) == 0);
    static_assert(N >= alignof(T));

    struct validated_t {};

    T* ptr_{};

    constexpr aligned_ptr(T* ptr, validated_t) noexcept : ptr_(ptr) {}

    template<class, std::size_t>
    friend class aligned_ptr;

public:
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

    template<class U, std::size_t M>
        requires (M >= N) && std::is_convertible_v<U (*)[], T (*)[]>
    constexpr aligned_ptr(aligned_ptr<U, M> other) noexcept : ptr_(other.get()) {}

    [[nodiscard]] constexpr T* get() const noexcept {
        return ptr_;
    }

    static constexpr std::size_t alignment = N;
};

struct base {
    int value{};
};

struct derived : base {
    int extra{};
};

using a32 = aligned_ptr<int, 32>;
using a64 = aligned_ptr<int, 64>;
using ca32 = aligned_ptr<const int, 32>;
using d64 = aligned_ptr<derived, 64>;
using b64 = aligned_ptr<base, 64>;

static_assert(std::is_convertible_v<a64, a32>);
static_assert(std::is_convertible_v<a64, ca32>);
static_assert(!std::is_convertible_v<a32, a64>);
static_assert(!std::is_convertible_v<ca32, a32>);
static_assert(!std::is_convertible_v<d64, b64>);
static_assert(a64::alignment == 64);

} // namespace spike

int main() {
    alignas(64) std::array<int, 2> values{1, 2};

    const auto aligned = spike::a64::try_from(&values[0]);
    if (!aligned || aligned->get() != &values[0]) {
        return 1;
    }

    const auto misaligned = spike::a64::try_from(&values[1]);
    if (misaligned) {
        return 2;
    }

    const auto null_value = spike::a64::try_from(nullptr);
    if (!null_value || null_value->get() != nullptr) {
        return 3;
    }

    const spike::a32 weaker = *aligned;
    if (weaker.get() != &values[0]) {
        return 4;
    }

    const spike::ca32 const_weaker = *aligned;
    if (const_weaker.get() != &values[0]) {
        return 5;
    }

    return 0;
}
