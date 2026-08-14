// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/aligned_ptr.hpp>

template <class T> void consume(precept::aligned_ptr<const T, 32>);

void reject(precept::aligned_ptr<int, 64> source) { consume(source); }
