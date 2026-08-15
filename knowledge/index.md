---
okf_version: "0.2"
---

# Vision

* [Project Charter](vision/project-charter.md) - Why Precept exists, what it optimizes for, and the scope of v0.1.

# Updates

* [Update Log](log.md) - Curated release and significant knowledge updates.

# Architecture

* [Design Principles](architecture/design-principles.md) - The principles that shape how Precept types, factories, and conversions are designed.
* [API Admission Rules](architecture/api-admission-rules.md) - The eight rules a public API must satisfy before it is accepted.

# API

* [v0.1 Span Family API Contract](api/span-family.md) - Guarantees, construction boundaries, conversions, lifetime rules, and operations for the four v0.1 span APIs.
* [aligned_ptr API Contract](api/aligned-ptr.md) - Alignment guarantee, nullable validation result, safe weakening, ownership boundary, and standard-pointer escape.
* [set_once API Contract](api/set-once.md) - One-way slot transition, observation, copy and move, failure, ownership, and contained-value boundaries.
* [nonzero API Contract](api/nonzero.md) - Non-zero integral guarantee, admitted element types, validation and observation boundaries, and the arithmetic propagation boundary.
* [narrow_exact API Contract](api/narrow-exact.md) - Exact integer representability rule, admitted integer types, failure model, and the standard-type return boundary.

# Rules

* [Coding Rules](rules/coding.md) - Source-file licensing and code-level conventions.
* [Documentation Rules](rules/documentation.md) - Language policy, README vs. knowledge split, knowledge authoring conventions, and how knowledge findings are graded.
* [AI-Assisted Development Rules](rules/ai-assisted-development.md) - How Codex and Claude Code participate, and how knowledge provenance is recorded.

# Decisions

* [ADR-0001: Use C++20 as the minimum language version](decisions/adr-0001-cpp20.md) - Use `std::span` directly instead of shipping a compatibility span.
* [ADR-0002: Ship as a header-only library with zero consumer dependencies](decisions/adr-0002-header-only.md) - Keep adoption cost close to copying a header.
* [ADR-0003: License Precept under Apache-2.0](decisions/adr-0003-apache-2-0.md) - Permissive license with an explicit patent grant.
* [ADR-0004: Prefer a dedicated semantic vocabulary over a generic constrained framework](decisions/adr-0004-dedicated-semantic-vocabulary.md) - Named types instead of a predicate DSL.
* [ADR-0005: Limit v0.1 to the span precondition family](decisions/adr-0005-v0-1-span-scope.md) - Validate one hypothesis before growing the vocabulary.
* [ADR-0006: Define validation and conversion boundaries for the v0.1 span family](decisions/adr-0006-validation-and-conversion-boundaries.md) - Preserve compile-time facts while distinguishing safe semantic weakening from invariant bypass.
* [ADR-0007: Define versioning, compatibility, deprecation, and platform support for v0.1.0](decisions/adr-0007-versioning-compatibility-and-support.md) - Adopt SemVer with a MINOR-boundary compatibility promise, a one-MINOR deprecation notice, and CI-verified platform support.

# Testing

* [Test Strategy](testing/test-strategy.md) - How runtime, compile-time, integration, and deferred tests are separated behind CTest.
