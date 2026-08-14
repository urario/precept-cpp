# Examples

Each example is a self-contained program that a consumer could paste into their own project. They
are built and executed by an ordinary test run, so they cannot drift away from the API.

* [`packet_parsing.cpp`](packet_parsing.cpp) — a minimum-size header precondition stated in the
  parameter type with `at_least_span`, and the exact-size counterpart, `checked_span`.
* [`fixed_block_processing.cpp`](fixed_block_processing.cpp) — `block_span` iteration with no tail
  case, where a partial final block fails validation instead of reaching the loop.
* [`non_empty_processing.cpp`](non_empty_processing.cpp) — `non_empty_span` making `front()` and
  `back()` guaranteed, and the guarantee being reused by a deeper function.

Build and run them from the repository root:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build -R '^example_' --output-on-failure
```

The exact contracts these examples rely on are defined in the
[v0.1 Span Family API Contract](../knowledge/api/span-family.md).
