// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Comparing a reusable non-decreasing transition with closed and domain-specific alternatives.
//
// `never_decrease` is useful here only when several update sites share the same meaning: an
// authoritative processed count may stay equal or advance, but it must not regress. High-water
// marks instead usually want `std::max`, while revisions often need stricter domain rules than
// non-decrease.

#include <precept/never_decrease.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace {

using processed_count = precept::never_decrease<std::size_t>;

// These alternative reporting paths share one authoritative cumulative count without repeating a
// setter check. A regression is an invalid report here, so the failure is propagated instead of
// treated as telemetry.
bool primary_path_report(processed_count& processed, std::size_t count) noexcept {
  return processed.try_update(count);
}

bool replay_path_report(processed_count& processed, std::size_t count) noexcept {
  return processed.try_update(count);
}

std::size_t reported_count(const processed_count& processed) noexcept { return processed.value(); }

// A high-water mark consumes every candidate by keeping the greatest value. Rejecting a lower
// candidate would be a different policy, so the closed operation is clearer here.
void observe_high_water(std::size_t& current, std::size_t candidate) noexcept {
  current = (std::max)(current, candidate);
}

struct configuration_revision {
  std::uint64_t number;
};

// A revision has domain-specific duplicate semantics: equal revisions are not new revisions.
// Conflict handling and skipped-number policy would also belong here, not in a generic ordering
// carrier.
bool accept_revision(configuration_revision& current, configuration_revision candidate) noexcept {
  if (candidate.number <= current.number) {
    return false;
  }

  current = candidate;
  return true;
}

// When the rule has only one local consumer, a raw value and a local check are easier to read.
bool set_local_progress(std::size_t& current, std::size_t next) noexcept {
  if (next < current) {
    return false;
  }

  current = next;
  return true;
}

} // namespace

int main() {
  processed_count processed{0};
  if (!primary_path_report(processed, 10) || !replay_path_report(processed, 20) ||
      reported_count(processed) != 20) {
    return 1;
  }

  // A stale primary-path report is rejected before the coordinator forwards it downstream.
  if (primary_path_report(processed, 15) || reported_count(processed) != 20) {
    return 2;
  }

  std::size_t high_water = 0;
  observe_high_water(high_water, 10);
  observe_high_water(high_water, 5); // Lower candidates are ignored, not rejected.
  observe_high_water(high_water, 20);
  if (high_water != 20) {
    return 3;
  }

  precept::never_decrease<std::uint64_t> generic_revision{7};
  if (!generic_revision.try_update(7)) {
    return 4;
  }

  configuration_revision revision{7};
  if (accept_revision(revision, configuration_revision{7}) ||
      !accept_revision(revision, configuration_revision{9}) || revision.number != 9) {
    return 5;
  }

  std::size_t local_progress = 0;
  if (!set_local_progress(local_progress, 5) || set_local_progress(local_progress, 4) ||
      local_progress != 5) {
    return 6;
  }

  return 0;
}
