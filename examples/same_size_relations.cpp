// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Compare local validation, a reusable same-size carrier, domain correspondence, and N-way use.

#include <precept/same_size.hpp>

#include <array>
#include <cstddef>
#include <optional>
#include <span>

namespace {

// One-shot work is clearer when the operation owns the cheap validation and consumes the spans.
std::optional<int> dot_product_local(std::span<const int> lhs, std::span<const int> rhs) noexcept {
  if (lhs.size() != rhs.size()) {
    return std::nullopt;
  }

  int result = 0;
  for (std::size_t index = 0; index < lhs.size(); ++index) {
    result += lhs[index] * rhs[index];
  }
  return result;
}

// This is intentionally the less natural one-shot alternative: the carrier is immediately
// unwrapped, so it does not add an API boundary or preserve a fact for another consumer.
std::optional<int> dot_product_carrier(std::span<const int> lhs,
                                       std::span<const int> rhs) noexcept {
  const auto pair = precept::checked_same_size(lhs, rhs);
  if (!pair) {
    return std::nullopt;
  }

  int result = 0;
  for (std::size_t index = 0; index < pair->size(); ++index) {
    result += pair->first()[index] * pair->second()[index];
  }
  return result;
}

// A multi-stage pipeline is the positive candidate: the relation is part of the signatures that
// consume both spans, so those stages do not rely on a caller convention or repeat a guard clause.
using paired_buffers = precept::same_size_pair<float, const float>;

void normalize(paired_buffers buffers) noexcept {
  for (std::size_t index = 0; index < buffers.size(); ++index) {
    buffers.first()[index] *= buffers.second()[index];
  }
}

void transform(std::span<float> values) noexcept {
  for (float& value : values) {
    value += 1.0F;
  }
}

float emit(paired_buffers buffers) noexcept {
  float total = 0.0F;
  for (std::size_t index = 0; index < buffers.size(); ++index) {
    total += buffers.first()[index] - buffers.second()[index];
  }
  return total;
}

std::optional<float> process_in_stages(std::span<float> values,
                                       std::span<const float> weights) noexcept {
  const auto pair = precept::checked_same_size(values, weights);
  if (!pair) {
    return std::nullopt;
  }

  normalize(*pair);
  transform(pair->first());
  return emit(*pair);
}

struct Job {
  int id;
};

struct Result {
  int job_id;
  int value;
};

// Equal cardinality is not correspondence. The generic carrier accepts this pair even though
// the first result belongs to a different job.
class job_results {
public:
  [[nodiscard]] static std::optional<job_results> try_from(std::span<const Job> jobs,
                                                           std::span<const Result> results) {
    if (jobs.size() != results.size()) {
      return std::nullopt;
    }
    for (std::size_t index = 0; index < jobs.size(); ++index) {
      if (jobs[index].id != results[index].job_id) {
        return std::nullopt;
      }
    }
    return job_results(jobs, results);
  }

  [[nodiscard]] std::span<const Job> jobs() const noexcept { return jobs_; }
  [[nodiscard]] std::span<const Result> results() const noexcept { return results_; }

private:
  job_results(std::span<const Job> jobs, std::span<const Result> results) noexcept
      : jobs_(jobs), results_(results) {}

  std::span<const Job> jobs_;
  std::span<const Result> results_;
};

bool domain_correspondence_example() {
  constexpr std::array jobs = {Job{10}, Job{20}};
  constexpr std::array mismatched_results = {Result{99, 1}, Result{20, 2}};
  constexpr std::array matching_results = {Result{10, 1}, Result{20, 2}};

  const auto structural_pair = precept::checked_same_size(
      std::span<const Job>{jobs}, std::span<const Result>{mismatched_results});
  if (!structural_pair || job_results::try_from(std::span<const Job>{jobs},
                                                std::span<const Result>{mismatched_results})) {
    return false;
  }

  return job_results::try_from(std::span<const Job>{jobs},
                               std::span<const Result>{matching_results})
      .has_value();
}

// Three parallel sequences create N-way pressure. Keep this as an ordinary local check; this
// experiment does not grow a triplet, nested carrier, or variadic relation framework.
bool apply_weighted_mask(std::span<float> values, std::span<const float> weights,
                         std::span<const bool> mask) noexcept {
  if (values.size() != weights.size() || values.size() != mask.size()) {
    return false;
  }

  for (std::size_t index = 0; index < values.size(); ++index) {
    if (mask[index]) {
      values[index] *= weights[index];
    }
  }
  return true;
}

} // namespace

int main() {
  constexpr std::array first = {1, 2, 3};
  constexpr std::array second = {4, 5, 6};

  if (dot_product_local(first, second) != dot_product_carrier(first, second) ||
      dot_product_local(first, second) != 32) {
    return 1;
  }

  std::array values = {1.0F, 2.0F, 3.0F};
  constexpr std::array weights = {0.5F, 1.0F, 2.0F};
  if (!process_in_stages(values, weights)) {
    return 2;
  }

  if (!domain_correspondence_example()) {
    return 3;
  }

  std::array mask = {true, false, true};
  if (!apply_weighted_mask(values, weights, mask)) {
    return 4;
  }

  return 0;
}
