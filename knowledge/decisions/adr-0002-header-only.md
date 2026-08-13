---
type: Architecture Decision
title: Ship as a header-only library with zero consumer dependencies
description: Distribute Precept as headers only, so adoption costs no build system changes and no transitive dependencies.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-13T00:00:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
  - id: issue-2
    resource: https://github.com/urario/precept-cpp/issues/2
    title: Foundation issue for the C++20 header-only build, test, and CI environment
    author: human:urario
  - id: issue-9
    resource: https://github.com/urario/precept-cpp/issues/9
    title: Issue for the CMake package and consumer smoke test
    author: human:urario
tags: [architecture, header-only, dependencies, packaging]
---

# Context

Precept is a small semantic vocabulary layered on standard types. Its value per line of
code is high, but the amount of code is deliberately tiny.

For a library this small, adoption friction dominates. A compiled artifact, a package
manager requirement, or a transitive dependency can easily cost a team more than writing
the guard clause by hand — which is exactly the comparison Precept loses if it is not
trivial to adopt.

# Decision

Precept is distributed as a header-only library with zero consumer dependencies. Consumers
need only a C++20 compiler and the headers.

Development-time tooling — test frameworks, formatters, static analyzers, knowledge
checks — must never be visible to consumers, neither as build requirements nor as
usage requirements on the exported target.

# Alternatives considered

Issue #1 records header-only and zero consumer dependencies as base policy, but not the
options weighed against them. The alternatives below were reconstructed while drafting this
ADR and are open to correction in review.

* **Compiled static or shared library** — allows hiding implementation and reduces compile
  time in large builds, but there is essentially no implementation to hide: the types are
  thin views and constexpr-friendly factories. Rejected as pure overhead.
* **Header-only, but depending on a third-party utility library** — would speed up early
  development, but every consumer inherits that dependency's version constraints and
  license. Rejected.
* **Module-first distribution (C++20 modules)** — attractive long term, but toolchain
  support across GCC, Clang, and MSVC is still uneven, and it would raise the adoption bar
  Precept is trying to keep low. Not chosen now; may be revisited additively.

# Consequences

* Everything in the public API lives in headers, so implementation changes are ABI-visible
  and header hygiene matters: no macro pollution, no unnamed-namespace state in headers.
* Compile-time cost is borne by consumers, which reinforces the "Tiny" admission rule in
  [API Admission Rules](../architecture/api-admission-rules.md).

## Requirements this places on the build system

Nothing below exists in the repository yet. These are requirements the build and packaging
work must satisfy when it is implemented in
[#2](https://github.com/urario/precept-cpp/issues/2) and
[#9](https://github.com/urario/precept-cpp/issues/9) — they are not descriptions of the
current repository state.

* The exported CMake target is an `INTERFACE` target carrying only include requirements and
  `cxx_std_20`.
* Development warnings, sanitizers, and test dependencies stay in the in-tree build and are
  never exported to consumers.
* A consumer smoke test confirms the zero-dependency claim instead of assuming it.

# Status

Accepted.

# Related

* Issue [#1](https://github.com/urario/precept-cpp/issues/1) — roadmap and base policy
* Issue [#2](https://github.com/urario/precept-cpp/issues/2) — build, test, and CI foundation
* Issue [#9](https://github.com/urario/precept-cpp/issues/9) — CMake package and consumer test
* [ADR-0001: Use C++20 as the minimum language version](adr-0001-cpp20.md)
