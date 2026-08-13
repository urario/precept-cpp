---
type: Architecture Decision
title: Ship as a header-only library with zero consumer dependencies
description: Distribute Precept as headers only, so adoption costs no build system changes and no transitive dependencies.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-13T19:24:27Z
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

The source material does not record alternatives considered for this decision.
No alternatives are reconstructed here.

# Consequences

* Everything in the public API lives in headers, so implementation changes are ABI-visible and
  header hygiene is part of the public contract: no macro pollution, no mutable per-translation-unit
  state, and nothing whose per-translation-unit identity or address can be observed through the
  public API or violate the ODR when a header is included from several translation units. This
  constrains state and identity that reach public semantics, not where implementation code is
  written — a header is the only place Precept has to write it.
* Compile-time cost is borne by consumers, which reinforces the "Tiny" admission rule in
  [API Admission Rules](../architecture/api-admission-rules.md).

## Constraints this places on the build and packaging work

These are standing constraints on build, test, and packaging changes rather than a description of
what the repository contains at any given moment. What is implemented, and where, is visible in
the build files, the tests, and the issues that own them —
[#2](https://github.com/urario/precept-cpp/issues/2) and
[#9](https://github.com/urario/precept-cpp/issues/9).

* The consumer-facing target carries the minimum a consumer needs — include requirements and the
  C++20 language requirement — and nothing that only the in-tree build cares about.
* Development tools are not consumer dependencies. Test frameworks, warning and sanitizer
  settings, formatters, analyzers, and knowledge tooling stay private to the development build and
  are never usage requirements.
* A configuration that only consumes the library neither requires nor fetches any of that tooling.
* Consumer isolation is confirmed by a test rather than asserted. A zero-dependency claim decays
  silently, so it is checked the same way behavior is.

# Status

Accepted.

# Related

* Issue [#1](https://github.com/urario/precept-cpp/issues/1) — roadmap and base policy
* Issue [#2](https://github.com/urario/precept-cpp/issues/2) — build, test, and CI foundation
* Issue [#9](https://github.com/urario/precept-cpp/issues/9) — CMake package and consumer test
* [ADR-0001: Use C++20 as the minimum language version](adr-0001-cpp20.md)
