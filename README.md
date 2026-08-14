# Precept

Tiny C++ types and operations that make semantic preconditions explicit.

Precept is a C++20, header-only library with no consumer dependencies. The public API is still
under development.

## Minimum-size spans

`at_least_span<T, N>` keeps a validated `size() >= N` fact visible in an API signature and carries
it across calls. Runtime validation is non-throwing and does not own the underlying storage:

```cpp
#include <precept/span/at_least_span.hpp>

#include <cstddef>
#include <span>
#include <vector>

void parse(precept::at_least_span<const std::byte, 16> packet)
{
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
source of truth for them. `tools/check_knowledge.py` checks that bundle and runs under CTest
alongside everything else, when Python and PyYAML are available; see
[CONTRIBUTING.md](CONTRIBUTING.md#knowledge-check) for the setup that guarantees it runs.

## Contributing

Contributions are welcome — see [CONTRIBUTING.md](CONTRIBUTING.md)
([Japanese translation](CONTRIBUTING.ja.md)). Issues and pull requests are written primarily in
Japanese; English is very welcome.

## License

Precept is licensed under the [Apache License 2.0](LICENSE). Source files carry the SPDX header
rather than the full notice; see the [Coding Rules](knowledge/rules/coding.md).
