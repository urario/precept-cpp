#include "candidate.hpp"

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace {
using a32 = spike::aligned_ptr<int, 32>;
using a64 = spike::aligned_ptr<int, 64>;
using ca32 = spike::aligned_ptr<const int, 32>;

struct base {
  int value{};
};
struct derived : base {
  int extra{};
};
using derived64 = spike::aligned_ptr<derived, 64>;
using base64 = spike::aligned_ptr<base, 64>;

static_assert(std::is_convertible_v<a64, a32>);
static_assert(std::is_convertible_v<a64, ca32>);
static_assert(!std::is_convertible_v<a32, a64>);
static_assert(!std::is_convertible_v<ca32, a32>);
static_assert(!std::is_convertible_v<derived64, base64>);
static_assert(sizeof(a64) == sizeof(int*));
static_assert(alignof(a64) == alignof(int*));
static_assert(std::is_trivially_copyable_v<a64>);

[[maybe_unused]] int select(ca32) { return 1; }
[[maybe_unused]] long select(const int*) { return 2; }
static_assert(std::is_same_v<decltype(select(std::declval<a64>())), int>);

template <class T, std::size_t N>
constexpr std::size_t observed_alignment(spike::aligned_ptr<T, N>) noexcept {
  return N;
}

void consume32(ca32) {}
} // namespace

int main() {
  alignas(64) std::array<int, 2> values{1, 2};

  const auto aligned = a64::try_from(&values[0]);
  if (!aligned || aligned->get() != &values[0]) {
    return 1;
  }

  const auto misaligned = a64::try_from(&values[1]);
  if (misaligned) {
    return 2;
  }

  const auto null_value = a64::try_from(nullptr);
  if (!null_value || null_value->get() != nullptr) {
    return 3;
  }

  consume32(*aligned);
  if (observed_alignment(*aligned) != 64) {
    return 4;
  }

  return 0;
}
