# Precept

Tiny C++ types and operations that make semantic preconditions explicit.

A signature such as `void parse(std::span<const std::byte> packet)` hides its most important design
condition: `packet.size() >= 16`. Every function downstream re-checks it, comments it, or trusts
human memory. Precept states it where callers can see it, and the fact travels with the value:

```cpp
void parse(precept::at_least_span<const std::byte, 16> packet);
```

Precept is a C++20, header-only library with no consumer dependencies. The v0.1 span family is
implemented. Tagged releases follow [Semantic Versioning](knowledge/decisions/adr-0007-versioning-compatibility-and-support.md):
within the 0.x series, PATCH releases preserve compatibility within a MINOR version, while a MINOR
release may break it. A public name is deprecated for at least one MINOR release before removal.

## Why this matters after generative AI

Guard clauses and boilerplate are cheap to generate now, so saving a few lines is not the point.
What Precept buys is different:

* Design assumptions stay visible in the API signature instead of inside a function body.
* An edit — written by a person or generated — is less likely to silently drop a precondition.
* A fact the caller already validated is reusable at deeper layers, not re-checked at each one.
* The condition is enforced by the type system, not by a comment that can go stale.

The reasoning behind this is recorded in the
[project charter](knowledge/vision/project-charter.md).

## Installation

Precept needs a C++20 compiler and, for the CMake integrations below, CMake 3.21 or newer. It has
no dependencies of its own, so nothing else is fetched into your build.

With `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
  precept
  GIT_REPOSITORY https://github.com/urario/precept-cpp.git
  GIT_TAG v0.1.0)
FetchContent_MakeAvailable(precept)

target_link_libraries(your_app PRIVATE Precept::Precept)
```

With a vendored copy of the repository:

```cmake
add_subdirectory(third_party/precept-cpp)

target_link_libraries(your_app PRIVATE Precept::Precept)
```

Either way, `Precept::Precept` is an interface target: it adds the include directory and requires
C++20, and it never pulls test dependencies or development tooling into your build. Because the
library is header-only, copying `include/` into your own include path works too.

To install the package from a source checkout:

```sh
cmake -S . -B build -DBUILD_TESTING=OFF
cmake --build build
cmake --install build --prefix <prefix>
```

An installed package can be consumed with the same target name:

```cmake
find_package(Precept 0.1 CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE Precept::Precept)
```

Configure the consumer with `-DCMAKE_PREFIX_PATH=<prefix>` when the install prefix is not already
known to CMake.

## Minimum-size spans

`at_least_span<T, N>` keeps a validated `size() >= N` fact visible in an API signature and carries
it across calls. Runtime validation is non-throwing and does not own the underlying storage:

```cpp
#include <precept/span/at_least_span.hpp>

#include <cstddef>
#include <span>
#include <vector>

void parse(precept::at_least_span<const std::byte, 16> packet) {
  std::span<const std::byte, 16> header = packet.prefix();
  std::span<const std::byte> payload = packet.rest();
  // Use header and payload without checking the minimum size again.
}

std::vector<std::byte> bytes = receive();
if (auto packet = precept::at_least_span<const std::byte, 16>::try_from(
        std::span<const std::byte>{bytes})) {
  parse(*packet);
}
```

For the common one-or-more case, `non_empty_span<T>` is the same type as
`at_least_span<T, 1>`. Once validation succeeds, `front()` and `back()` are guaranteed:

```cpp
if (auto values = precept::non_empty_span<int>::try_from(input)) {
  use(values->front(), values->back());
}
```

`at_least_span` truthfully models a contiguous, sized, borrowed range, so it can safely weaken to
a compatible dynamic-extent `std::span` when an API does not require the minimum-size fact.
`as_span()` remains the readable named form when you want that standard-span use to be explicit.
Use `prefix()`—not the whole view—when you need the guaranteed `std::span<T, N>` prefix.

## Exact-size spans

`checked_span<N>` validates an exact runtime size and returns the fact as the standard fixed-extent
`std::span<T, N>`. A mismatch returns `std::nullopt` without truncating the input:

```cpp
#include <precept/span/checked_span.hpp>

#include <cstddef>
#include <span>

void consume_header(std::span<const std::byte, 16> header);

std::span<const std::byte> input = receive_view();
if (auto header = precept::checked_span<16>(input)) {
  consume_header(*header);
}
```

The factory accepts `std::span` only. Construct a span explicitly before validating a container.

## Fixed-size block spans

`block_span<T, N>` validates that an element sequence contains only complete `N`-element blocks.
Iteration returns each block as a fixed-extent `std::span<T, N>`, so consumers do not need a tail
case:

```cpp
#include <precept/span/block_span.hpp>

#include <cstddef>
#include <span>

std::span<const std::byte> input = receive_view();
if (auto blocks = precept::block_span<const std::byte, 16>::try_from(input)) {
  for (std::span<const std::byte, 16> block : *blocks) {
    consume(block);
  }
}
```

`size()` and `block_count()` report the number of logical blocks. Use `as_span().size()` when you
need the underlying element count. Empty input is valid, while a partial final block returns
`std::nullopt`.

## Aligned pointers

`aligned_ptr<T, N>` carries a validated current-address alignment fact across API boundaries
without owning the pointed-to object or becoming a range:

```cpp
#include <precept/aligned_ptr.hpp>

void process(precept::aligned_ptr<const float, 32> input);

if (auto input = precept::aligned_ptr<float, 64>::try_from(data)) {
  process(*input); // safely weakens 64-byte mutable to 32-byte const
}
```

A non-null misaligned pointer produces `std::nullopt`. Null is an ordinary valid pointer state, so
it produces an engaged optional whose `get()` is null. `get()` is the explicit standard-pointer
escape and does not hide `std::assume_aligned`; a downstream facility keeps its own preconditions.
The exact boundary is defined by the
[`aligned_ptr` API contract](knowledge/api/aligned-ptr.md).

## Set-once values

`set_once<T>` owns a value whose slot can move from unset to set at most once. It carries a
one-way initialization rule across API boundaries without becoming a synchronization primitive:

```cpp
#include <precept/set_once.hpp>

struct configuration {
  explicit configuration(int workers) : worker_count(workers) {}

  int worker_count;
};

precept::set_once<configuration> config;
if (config.try_emplace(4)) {
  start_workers(config.value().worker_count);
}
```

`try_set()` and `try_emplace()` return `false` after the first successful setting attempt, without
copying, moving, or constructing the contained value again. `value()` returns `const T&` and throws
`std::bad_optional_access` while unset. The holder has no assignment, reset, replacement, mutable
observer, or optional-style operators.

The guarantee applies to slot assignment only. It does not make state reachable through `T`
deeply immutable, and it provides no thread synchronization. The exact boundary is defined by the
[`set_once` API contract](knowledge/api/set-once.md).

## Non-decreasing transitions

`never_decrease<T>` carries one object's integral value history across update sites. Equal values
are accepted, regressions are rejected, and the stored value is unchanged after rejection:

```cpp
#include <precept/never_decrease.hpp>

#include <cstddef>

precept::never_decrease<std::size_t> processed{0};
if (!processed.try_update(10) || !processed.try_update(20)) {
  return false; // The report could not advance the count.
}
if (!processed.try_update(15)) {
  return false; // Reject the invalid regression; the value is still 20.
}
```

This is an experimental transition vocabulary, not a general progress or revision abstraction.
Use `std::max` when a high-water mark should ignore lower candidates, and use a domain-specific
type when sequence or revision rules distinguish duplicates, conflicts, skipped values, or
wraparound. The type has no arithmetic or thread-safety support. See the runnable
[transition comparison example](examples/never_decrease_transitions.cpp) and the
[`never_decrease` API contract](knowledge/api/never-decrease.md).

## Non-zero integers

`nonzero<T>` carries a validated `value != 0` fact across API boundaries, for integral `T` other
than `bool`:

```cpp
#include <precept/nonzero.hpp>

std::size_t decimated_length(std::size_t input, precept::nonzero<std::uint32_t> decimation);

if (auto decimation = precept::nonzero<std::uint32_t>::try_from(raw)) {
  use(decimated_length(samples.size(), *decimation)); // no divisor check here or below
}
```

`try_from()` is `constexpr` and `noexcept`, `value()` is the only way back to a plain scalar, and
there is no implicit conversion in either direction.

`nonzero<T>` carries only the fact that the stored scalar is non-zero. It is not a general numeric
wrapper and does not propagate the guarantee through arbitrary arithmetic: the difference of two
non-zero values can be zero, so the type offers no arithmetic operators at all. Non-zero is also
not always sufficient by itself — signed division still requires a representable result, and a
validated index still needs its own range check. The exact boundary is defined by the
[`nonzero` API contract](knowledge/api/nonzero.md).

## Exact integer narrowing

`narrow_exact<T>(value)` converts one integer to another when `T` represents the value exactly,
and returns `std::nullopt` when it does not:

```cpp
#include <precept/narrow_exact.hpp>

std::size_t payload_size = payload.size();
if (auto wire_size = precept::narrow_exact<std::uint16_t>(payload_size)) {
  write_length_prefix(*wire_size); // an ordinary std::uint16_t from here on
}
```

Representability is decided on the value, so one rule covers both directions of the mistake: a
negative value never becomes a large unsigned one, and a value above the destination maximum is
never wrapped. The decision itself is `std::in_range`, which Precept does not reimplement.

There is no `narrow_exact` type. The verified fact is that the value fits in `T`, and a `T`
holding that value is that fact — nothing further needs to travel with it. The exact boundary,
including the admitted integer types and how this differs from `nonzero`, is defined by the
[`narrow_exact` API contract](knowledge/api/narrow-exact.md).

## Non-overlapping spans

`checked_non_overlapping(first, second)` validates that the object-representation byte ranges of
two `std::span` values share no byte and returns their typed span snapshots as a carrier:

```cpp
#include <precept/non_overlapping.hpp>

using separated_io = precept::non_overlapping_spans<std::byte, const std::byte>;

void stage_a(separated_io buffers);
void stage_b(separated_io buffers);

if (auto buffers = precept::checked_non_overlapping(output, input)) {
  stage_a(*buffers);
  stage_b(*buffers); // the same validated pair is reused
}
```

Empty ranges are accepted, adjacent ranges are non-overlapping, and different element types are
compared by byte extent. Validation is portable C++20 and linear in the sum of the byte extents; it
does not order unrelated pointers or convert them to integers. The carrier is a borrowed view and
follows the lifetime and invalidation rules of its stored spans.

For a one-shot operation such as a single copy, let that operation validate and consume the
relation locally. For input/output/scratch roles, a domain-specific aggregate is usually clearer
than three pair carriers. The runnable
[buffer example](examples/non_overlapping_buffers.cpp) demonstrates all three shapes, and the exact
boundary is defined by the
[`non_overlapping_spans` API contract](knowledge/api/non-overlapping.md).

## Experimental same-size span relations

`checked_same_size(first, second)` validates two dynamic `std::span` values and returns a
`same_size_pair` whose only guarantee is `first().size() == second().size()`:

```cpp
#include <precept/same_size.hpp>

using paired_values = precept::same_size_pair<float, const float>;

void normalize(paired_values values);
void emit(paired_values values);

if (auto values = precept::checked_same_size(raw_values, weights)) {
  normalize(*values);
  emit(*values); // the equal-size relation remains in both signatures
}
```

This is a v0.2 experimental relation vocabulary, not a general zip or correspondence proof.
Equal size does not imply that elements correspond semantically, share a domain or owner, or have
the same lifetime. Use a local check for one-shot work, and use a domain-specific aggregate when
roles or index identity matter. The [same-size relation example](examples/same_size_relations.cpp)
and [draft contract](knowledge/api/same-size.md) record the current usage boundary.

## Examples

[`examples/`](examples/) holds complete programs for the three span usages the v0.1 vocabulary was
designed around: [packet and header parsing](examples/packet_parsing.cpp),
[fixed-block processing](examples/fixed_block_processing.cpp), and
[non-empty collection processing](examples/non_empty_processing.cpp). The
[aligned buffer example](examples/aligned_buffer_processing.cpp) shows the first structural-property
carrier, the [set-once configuration example](examples/set_once_configuration.cpp) carries a
one-way initialization rule to a separate use site, and the
[decimation example](examples/nonzero_scaling.cpp) carries a validated divisor from a configuration
boundary into two deeper layers. The
[wire-field example](examples/narrow_exact_wire_fields.cpp) shows the opposite shape: validation
that ends in an ordinary integer rather than a Precept type. The
[non-overlapping buffer example](examples/non_overlapping_buffers.cpp) compares one-shot,
multi-stage, and role-bearing scratch-buffer contracts. The
[transition comparison example](examples/never_decrease_transitions.cpp) shows where a
non-decreasing carrier is useful and where a closed operation, domain-specific type, or local
setter is clearer. The
[same-size relation example](examples/same_size_relations.cpp) compares local, multi-stage,
domain-specific, and N-way shapes. They are built and executed by an ordinary test run, so they
always match the current API.

## Scope and non-goals

v0.1 is deliberately narrow: size preconditions on spans, and nothing else. The point is to test
one hypothesis — that C++ developers want to state recurring `std::span` size preconditions as
parameter types and reuse them — before growing the vocabulary. See
[ADR-0005](knowledge/decisions/adr-0005-v0-1-span-scope.md).

Precept does not aim to:

* become a general-purpose constraint or predicate framework,
* replace or fully wrap standard library types,
* make borrowed-view APIs own or extend the lifetime of their storage,
* become a general-purpose owning container library,
* provide a C++17 compatibility span.

Candidates explored after v0.1, and the criteria a new API must satisfy, are listed in the
[project charter](knowledge/vision/project-charter.md) and the
[API admission rules](knowledge/architecture/api-admission-rules.md).

## Supported environments

A platform or compiler counts as supported when a required CI job verifies it — not because the
maintainer happens to own the hardware. See
[ADR-0007](knowledge/decisions/adr-0007-versioning-compatibility-and-support.md) for the full
policy. Every environment below is CI-verified, with no distinction drawn between them:

| OS | Compiler | Standard library | Verified version |
| --- | --- | --- | --- |
| Linux (ubuntu-24.04) | GCC | libstdc++ | GNU 13.3.0 |
| Linux (ubuntu-24.04) | Clang | libstdc++ (default) | Clang 18.1.3 |
| Linux (ubuntu-24.04) | Clang | libc++ | Clang 18.1.3, libc++ 18.1.3 |
| Windows (windows-2025) | MSVC | MSVC STL | MSVC 19.51.36252.0 (Visual Studio 18 2026) |
| macOS (macos-26-arm64) | AppleClang | libc++ | AppleClang 21.0.0.21000101 |

These are the versions the CI matrix's default runner images actually provide; no older toolchain
has been verified, so none is claimed as a minimum.

## Development

The supported development entry point is CMake and CTest. Configuring tests for the first time
requires network access so CMake can fetch the pinned GoogleTest release.

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

[CONTRIBUTING.md](CONTRIBUTING.md) covers the rest: library-only and multi-configuration builds,
the knowledge check, formatting, what a change is reviewed against, and how to open a pull request.

## Knowledge base

Design decisions, rules, and ADRs live in the [knowledge bundle](knowledge/index.md), which is the
source of truth for them. Start from [`knowledge/index.md`](knowledge/index.md); the decisions that
shape everything else are:

Release notes are recorded in the [update log](knowledge/log.md).

* [ADR-0001](knowledge/decisions/adr-0001-cpp20.md) — C++20 as the minimum language version.
* [ADR-0002](knowledge/decisions/adr-0002-header-only.md) — header-only, zero consumer
  dependencies.
* [ADR-0004](knowledge/decisions/adr-0004-dedicated-semantic-vocabulary.md) — a dedicated
  vocabulary rather than a generic constraint framework.
* [ADR-0005](knowledge/decisions/adr-0005-v0-1-span-scope.md) — the v0.1 scope boundary.

The exact guarantees, conversions, and failure model of the four APIs above are defined by the
[v0.1 span family API contract](knowledge/api/span-family.md).

`tools/check_knowledge.py` checks the bundle and runs under CTest alongside everything else, when
Python and PyYAML are available; see [CONTRIBUTING.md](CONTRIBUTING.md#knowledge-check) for the
setup that guarantees it runs.

## Contributing

Contributions are welcome — see [CONTRIBUTING.md](CONTRIBUTING.md)
([Japanese translation](CONTRIBUTING.ja.md)). Issues and pull requests are written primarily in
Japanese; English is very welcome.

## License

Precept is licensed under the [Apache License 2.0](LICENSE). Source files carry the SPDX header
rather than the full notice; see the [Coding Rules](knowledge/rules/coding.md).
