// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Non-empty collection processing.
//
// "At least one element" is the most common precondition there is, and the one most often left to
// a comment. `non_empty_span<T>` — the same type as `at_least_span<T, 1>` — puts it in the
// signature, so `front()` and `back()` are guaranteed to exist and an aggregation needs no
// identity value for the empty case.

#include <precept/span/non_empty_span.hpp>

#include <array>
#include <iostream>
#include <span>
#include <vector>

namespace {

struct temperature_range {
  int lowest;
  int highest;
};

// Neither the empty check nor a sentinel initial value is needed: `front()` is a real element, so
// it seeds the comparison directly.
temperature_range summarize(precept::non_empty_span<const int> readings) {
  temperature_range range{readings.front(), readings.front()};

  for (const int reading : readings) {
    range.lowest = reading < range.lowest ? reading : range.lowest;
    range.highest = reading > range.highest ? reading : range.highest;
  }
  return range;
}

// The guarantee is reusable: this function calls `summarize()` without re-validating anything.
void report(precept::non_empty_span<const int> readings) {
  const temperature_range range = summarize(readings);

  std::cout << "readings: " << readings.size() << ", latest: " << readings.back()
            << ", lowest: " << range.lowest << ", highest: " << range.highest << '\n';
}

} // namespace

int main() {
  const std::vector<int> readings = {21, 19, 27, 23};
  if (auto measured =
          precept::non_empty_span<const int>::try_from(std::span<const int>{readings})) {
    report(*measured);
  }

  // An empty sequence stops at the boundary instead of reaching the aggregation.
  const std::vector<int> nothing;
  if (!precept::non_empty_span<const int>::try_from(std::span<const int>{nothing})) {
    std::cout << "no readings to summarize\n";
  }

  // A fixed-extent span already proves the size, so it converts implicitly with no runtime check.
  const std::array<int, 3> calibration = {5, 12, 8};
  report(std::span<const int, 3>{calibration});

  return 0;
}
