// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Carrying a one-way initialization rule from configuration to use sites.
//
// `set_once` owns the value and prevents replacement of its slot. It is not a synchronization
// primitive and does not claim that state reachable through the contained value is deeply
// immutable.

#include <precept/set_once.hpp>

namespace {

struct configuration {
  explicit configuration(int workers) : worker_count(workers) {}

  int worker_count;
};

int configured_workers(const precept::set_once<configuration>& config) {
  return config.value().worker_count;
}

} // namespace

int main() {
  precept::set_once<configuration> config;
  if (!config.try_emplace(4)) {
    return 1;
  }

  if (configured_workers(config) != 4) {
    return 2;
  }

  if (config.try_emplace(8)) {
    return 3;
  }

  return configured_workers(config) == 4 ? 0 : 4;
}
