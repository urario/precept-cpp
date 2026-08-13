---
type: Architecture Decision
title: Ship as a header-only library with zero consumer dependencies
description: Distribute Precept as headers only, so adoption costs no build system changes and no transitive dependencies.
status: draft
generated:
  by: claude-code
  at: 2026-08-13T00:00:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
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

* The CMake target `Precept::Precept` is an `INTERFACE` target carrying only include
  requirements and `cxx_std_20`.
* Everything in the public API lives in headers, so implementation changes are ABI-visible
  and header hygiene matters: no macro pollution, no unnamed-namespace state in headers.
* Development warnings, sanitizers, and test dependencies stay in the in-tree build and are
  not exported.
* Compile-time cost is borne by consumers, which reinforces the "Tiny" admission rule in
  [API Admission Rules](/architecture/api-admission-rules.md).
* A consumer smoke test is needed to confirm the zero-dependency claim rather than assuming
  it.

# Status

Accepted.

# Related

* Issue [#1](https://github.com/urario/precept-cpp/issues/1) — roadmap and base policy
* Issue [#2](https://github.com/urario/precept-cpp/issues/2) — build, test, and CI foundation
* Issue [#9](https://github.com/urario/precept-cpp/issues/9) — CMake package and consumer test
* [ADR-0001: Use C++20 as the minimum language version](/decisions/adr-0001-cpp20.md)
