// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Carrying a validated divisor from a configuration boundary into deeper layers.
//
// The decimation factor is checked once, where the configuration is built from an external value.
// The planning layer divides by it and the kernel below strides by it; neither repeats the check,
// and neither can be reached with a zero factor.
//
// `nonzero` carries that one fact and nothing else. The kernel still states its own buffer-size
// precondition separately, and computing on `value()` produces ordinary integers again.

#include <precept/nonzero.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace {

using decimation_factor = precept::nonzero<std::uint32_t>;

struct stream_config {
  decimation_factor decimation;
};

// Planning layer: divides without re-checking, and without a comment asking callers not to pass 0.
std::size_t decimated_length(std::size_t input_length, decimation_factor decimation) noexcept {
  return input_length / decimation.value();
}

// Kernel: strides without re-checking. A zero step would not be undefined behavior here, it would
// simply never terminate, which is exactly the kind of assumption that survives only as a comment.
void decimate(std::span<const std::int32_t> input, std::span<std::int32_t> output,
              decimation_factor decimation) noexcept {
  const std::size_t step = decimation.value();
  std::size_t written = 0;
  for (std::size_t index = 0; index < input.size() && written < output.size(); index += step) {
    output[written++] = input[index];
  }
}

// Boundary: the only place the raw value is inspected.
std::optional<stream_config> parse_config(std::uint32_t raw_decimation) noexcept {
  const auto decimation = decimation_factor::try_from(raw_decimation);
  if (!decimation) {
    return std::nullopt;
  }

  return stream_config{*decimation};
}

} // namespace

int main() {
  if (parse_config(0)) {
    return 1;
  }

  const auto config = parse_config(4);
  if (!config) {
    return 2;
  }

  const std::array<std::int32_t, 12> samples{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  const std::size_t output_length = decimated_length(samples.size(), config->decimation);
  if (output_length != 3) {
    return 3;
  }

  std::array<std::int32_t, 3> decimated{};
  decimate(samples, std::span{decimated}.first(output_length), config->decimation);

  return decimated == std::array<std::int32_t, 3>{0, 4, 8} ? 0 : 4;
}
