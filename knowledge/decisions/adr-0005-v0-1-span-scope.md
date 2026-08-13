---
type: Architecture Decision
title: Limit v0.1 to the span precondition family
description: Release only span size preconditions in v0.1 so one hypothesis is tested before the vocabulary grows.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-13T00:00:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
tags: [architecture, scope, v0.1, span]
---

# Context

Many precondition families are plausible for Precept: alignment, numeric refinements,
transition semantics, semantic operations, and runtime protocol guards. Shipping several at
once would make the library look complete on day one.

It would also make the central question unanswerable. Precept's hypothesis is that
developers want recurring `std::span` size preconditions expressed as parameter types and
validated factories, and will reuse them. A broad first release mixes that signal with
feedback about unrelated families, and every published name becomes a compatibility
obligation.

# Decision

v0.1 contains only the span size precondition family:

```cpp
precept::at_least_span
precept::non_empty_span
precept::block_span
precept::checked_span(...)
```

`aligned_ptr`, numeric refinements (`finite`, `nonzero`, `power_of_two`), transition types
(`never_decrease`, `never_increase`, `late`), semantic operations (`parse_exact()`,
`zip_exact()`, `read_exact()`, `write_all()`), and runtime protocol guards (`no_overlap`,
`must_complete`, `latest::gate`) are out of scope for v0.1. Whether the runtime protocol
family belongs in Precept at all is re-evaluated after v0.1.

# Alternatives considered

Issue #1 records the scope itself, and that the other families are exploration candidates
after v0.1. It does not record a comparison of release strategies. The two alternatives
below were reconstructed while drafting this ADR and are open to correction in review; no
release or product strategy beyond the recorded scope is decided here.

* **Ship several families in v0.1** — a more impressive first release, but it multiplies
  unreviewed public API, dilutes the feedback signal, and locks in names before the core
  idea is validated. Rejected, consistent with the scope recorded in issue #1.
* **Ship only `at_least_span`** — the smallest possible test, but `checked_span()` and
  `block_span` are what demonstrate the two principles that make Precept more than an
  assertion helper: returning to standard types, and turning a constraint into a better
  API. Rejected as too narrow to test the real hypothesis.

# Consequences

* Requests for other families are expected and welcome; they are recorded as issues and
  evaluated against the [API Admission Rules](../architecture/api-admission-rules.md)
  after v0.1.
* Effort concentrates on the exact semantics of four APIs rather than the breadth of the
  surface, which matches the "few APIs, precise meaning" stance in issue #4.
* The README must state the scope and non-goals plainly, so v0.1 is not mistaken for an
  abandoned general-purpose library.
* A later family may reveal that a v0.1 name was too specific or too general. That risk is
  accepted; a small surface is cheaper to correct.

# Status

Accepted.

# Related

* Issue [#1](https://github.com/urario/precept-cpp/issues/1) — roadmap and base policy
* Issue [#4](https://github.com/urario/precept-cpp/issues/4) — v0.1 span API contract
* Issues [#5](https://github.com/urario/precept-cpp/issues/5),
  [#6](https://github.com/urario/precept-cpp/issues/6),
  [#7](https://github.com/urario/precept-cpp/issues/7) — v0.1 implementations
* [Project Charter](../vision/project-charter.md)
