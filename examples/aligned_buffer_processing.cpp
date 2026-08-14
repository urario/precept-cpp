// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Passing a validated alignment fact across an API boundary.
//
// The caller validates the current address once. The consumer receives that fact in its parameter
// type and decides where to apply the standard optimizer hint. Buffer length and lifetime remain
// separate caller responsibilities; `aligned_ptr` deliberately does not become a range.

#include <precept/aligned_ptr.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <numeric>

namespace {

float sum_aligned(precept::aligned_ptr<const float, 32> input, std::size_t count) {
  const float* data = input.get();
  if (data == nullptr) {
    return 0.0F;
  }

  data = std::assume_aligned<32>(data);
  return std::accumulate(data, data + count, 0.0F);
}

} // namespace

int main() {
  alignas(64) std::array<float, 8> samples = {1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F};

  const auto input = precept::aligned_ptr<float, 64>::try_from(samples.data());
  if (!input || sum_aligned(*input, samples.size()) != 36.0F) {
    return 1;
  }

  const auto empty = precept::aligned_ptr<float, 64>::try_from(nullptr);
  if (!empty || sum_aligned(*empty, 0) != 0.0F) {
    return 2;
  }

  if (precept::aligned_ptr<float, 64>::try_from(&samples[1])) {
    return 3;
  }

  return 0;
}
