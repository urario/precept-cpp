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

For multi-configuration generators such as Visual Studio, select the same configuration when
building and testing:

```sh
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

A library-only configuration does not fetch or build GoogleTest:

```sh
cmake -S . -B build-no-tests -DBUILD_TESTING=OFF
cmake --build build-no-tests
```

Runtime tests use GoogleTest, compile-time properties use `static_assert`, and CTest is the common
test execution entry point. Positive compile tests are normal CMake build targets. Negative compile
tests will be added only when a public API has constraints that need them.

## Knowledge base

Design decisions, rules, and ADRs live in the [knowledge bundle](knowledge/index.md), which is the
source of truth for them. `tools/check_knowledge.py` checks that bundle and runs under CTest with
everything else, so `ctest` covers the knowledge base as well as the code. It needs a Python 3
interpreter and PyYAML:

```sh
python -m pip install pyyaml
python tools/check_knowledge.py knowledge
```

Both are development-only and are never requirements of the library. When either is missing, CMake
says so at configure time and the knowledge tests are not registered; configuring with
`-DPRECEPT_REQUIRE_KNOWLEDGE_CHECK=ON`, as CI does, turns that into a configuration error instead.

## Formatting and static analysis

The repository `.clang-format` file defines the source formatting style. Check or update the
currently tracked C++ files with:

```sh
clang-format --dry-run --Werror tests/*.cpp
clang-format -i tests/*.cpp
```

clang-tidy is not a required quality gate yet because there are no production headers to analyze.
It will be reconsidered when the public API is introduced; project-wide analyzer settings must not
be imposed on consumers or fetched dependencies.
