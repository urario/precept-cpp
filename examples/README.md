# Examples

Each example is a self-contained program that a consumer could paste into their own project. They
are built and executed by an ordinary test run, so they cannot drift away from the API.

* [`packet_parsing.cpp`](packet_parsing.cpp) — a minimum-size header precondition stated in the
  parameter type with `at_least_span`, and the exact-size counterpart, `checked_span`.
* [`fixed_block_processing.cpp`](fixed_block_processing.cpp) — `block_span` iteration with no tail
  case, where a partial final block fails validation instead of reaching the loop.
* [`non_empty_processing.cpp`](non_empty_processing.cpp) — `non_empty_span` making `front()` and
  `back()` guaranteed, and the guarantee being reused by a deeper function.
* [`aligned_buffer_processing.cpp`](aligned_buffer_processing.cpp) — carrying a validated pointer
  alignment into a consumer that applies the standard optimizer hint.
* [`set_once_configuration.cpp`](set_once_configuration.cpp) — carrying one-way initialization
  from a configuration site to a separate use site without offering replacement or reset.
* [`nonzero_scaling.cpp`](nonzero_scaling.cpp) — a divisor validated once at a configuration
  boundary and reused by a planning layer and a kernel, neither of which re-checks it.
* [`narrow_exact_wire_fields.cpp`](narrow_exact_wire_fields.cpp) — in-memory values narrowed into
  the field widths of a wire format, where validation ends in ordinary integers rather than a
  Precept type.
* [`index_below_fixed_tables.cpp`](index_below_fixed_tables.cpp) — a fixed protocol field index
  reused by two consumers, contrasted with a one-shot lookup and loop that keep local raw indices.
* [`non_overlapping_buffers.cpp`](non_overlapping_buffers.cpp) - one-shot, multi-stage, and scratch
  buffer processing compared to show when a non-overlap carrier survives long enough to help.
* [`never_decrease_transitions.cpp`](never_decrease_transitions.cpp) - a non-decreasing processed
  count compared with `std::max` for a high-water mark, a domain-specific revision, and a local
  setter.
* [`same_size_relations.cpp`](same_size_relations.cpp) - one-shot, multi-stage, domain-correspondence,
  and three-sequence experiments that define the boundary of the experimental same-size carrier.

Build and run them from the repository root:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build -R '^example_' --output-on-failure
```

The exact contracts these examples rely on are defined in the
[v0.1 Span Family API Contract](../knowledge/api/span-family.md),
[aligned_ptr API Contract](../knowledge/api/aligned-ptr.md),
[set_once API Contract](../knowledge/api/set-once.md),
[nonzero API Contract](../knowledge/api/nonzero.md),
[index_below API Contract](../knowledge/api/index-below.md),
[narrow_exact API Contract](../knowledge/api/narrow-exact.md), and the
[non-overlapping spans API Contract](../knowledge/api/non-overlapping.md), and the
[never_decrease API Contract](../knowledge/api/never-decrease.md).
[non-overlapping spans API Contract](../knowledge/api/non-overlapping.md), and the experimental
[same-size span relation API Contract](../knowledge/api/same-size.md).
