---
type: Design Principle
title: Precept Design Principles
description: The principles that shape how Precept types, factories, and conversions are designed.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-13T00:00:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
  - id: issue-4
    resource: https://github.com/urario/precept-cpp/issues/4
    title: Design issue for the v0.1 span API contract, failure model, and conversions
    author: human:urario
tags: [architecture, design-principles]
---

# Scope of this document

These principles apply to every public Precept API. They describe *how* to design an API,
while [API Admission Rules](api-admission-rules.md) describe *whether* an API
is accepted at all.

# Build thinly on the standard library

Precept sits on top of the STL and does not replace it. Standard types remain the common
currency between libraries; Precept adds a thin semantic layer over them.

# Carry the fact, not just the check

The value of Precept is not a shorter guard clause. It is that a semantic fact, verified
once, is preserved by the type and reusable across API boundaries.

# Prefer a dedicated vocabulary over a generic framework

Small names that explain themselves are preferred over a composable predicate DSL:

```cpp
// Preferred
precept::at_least_span<const std::byte, 16>

// Not the direction of this project
refined_span<const std::byte, size_at_least<16>, size_multiple_of<8>>
```

See [ADR-0004](../decisions/adr-0004-dedicated-semantic-vocabulary.md).

# Factory to standard type

When a verified fact is fully expressible by a standard type, do not introduce a new
wrapper. Validate, then return the standard type.

```cpp
// A runtime-sized span checked to be exactly N elements is just a fixed-extent std::span.
std::optional<std::span<T, N>> checked_span(std::span<T> input);
```

New types are justified only when the invariant must survive beyond the check.

# Constraint to better API

A constraint should improve how callers write code, not merely record a fact. `block_span`
exists because it lets callers iterate fixed-size blocks without tail handling — not
because `size() % N == 0` is worth recording on its own.

# Invariants must not be bypassable

If a type exists, its invariant holds. Every public construction path — constructors,
factories, conversions, assignment — must preserve it. An invariant that can be sidestepped
is worse than no invariant, because readers stop trusting the signature.

# Views do not own storage

Precept types are borrowed views. They do not own, allocate, or extend the lifetime of the
underlying storage, and must not appear to do so. Lifetime rules follow those of the
standard view they wrap.

# No magic

No hidden global state, no implicit registration, no required runtime, no code generation.
What the signature says is what happens.

# Better than an assert

An API earns its place only if it does more than move an `assert` somewhere else: the
verified fact must be visible in the signature and reusable downstream.

# Open questions owned elsewhere

The following are deliberately **not** decided here:

* The validation failure model (for example whether factories return `std::optional`),
  `constexpr`/`noexcept` policy, CTAD, implicit vs. explicit conversions, and the exact
  contracts of the v0.1 span family — owned by issue #4 and to be recorded later as an
  `API Contract` concept, plus an ADR if the decision is expensive to reverse.
* Whether unchecked construction is exposed publicly at all — also part of issue #4.
