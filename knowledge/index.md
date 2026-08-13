---
okf_version: "0.2"
---

# Vision

* [Project Charter](vision/project-charter.md) - Why Precept exists, what it optimizes for, and the scope of v0.1.

# Architecture

* [Design Principles](architecture/design-principles.md) - The principles that shape every Precept API.
* [API Admission Rules](architecture/api-admission-rules.md) - The eight rules a public API must satisfy before it is accepted.

# Rules

* [Documentation Rules](rules/documentation.md) - Language policy, README vs. knowledge split, and knowledge authoring conventions.
* [AI-Assisted Development Rules](rules/ai-assisted-development.md) - How Codex and Claude Code participate, and how knowledge provenance is recorded.

# Decisions

* [ADR-0001: Use C++20 as the minimum language version](decisions/adr-0001-cpp20.md) - Use `std::span` directly instead of shipping a compatibility span.
* [ADR-0002: Ship as a header-only library with zero consumer dependencies](decisions/adr-0002-header-only.md) - Keep adoption cost close to copying a header.
* [ADR-0003: License Precept under Apache-2.0](decisions/adr-0003-apache-2-0.md) - Permissive license with an explicit patent grant.
* [ADR-0004: Prefer a dedicated semantic vocabulary over a generic constrained framework](decisions/adr-0004-dedicated-semantic-vocabulary.md) - Named types instead of a predicate DSL.
* [ADR-0005: Limit v0.1 to the span precondition family](decisions/adr-0005-v0-1-span-scope.md) - Validate one hypothesis before growing the vocabulary.

# Testing

* [Test Strategy](testing/test-strategy.md) - How runtime, compile-time, integration, and deferred tests are separated behind CTest.
