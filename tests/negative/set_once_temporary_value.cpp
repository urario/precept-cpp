// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

#include <precept/set_once.hpp>

const int& invalid_reference() { return precept::set_once<int>{}.value(); }

int main() { return invalid_reference(); }
