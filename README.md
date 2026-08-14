# Precept

Tiny C++ types and operations that make semantic preconditions explicit.

Precept is a C++20, header-only library with no consumer dependencies. The public API is still
under development.

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
