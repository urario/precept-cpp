---
type: Design Principle
title: Precept Design Principles
description: The principles that shape how Precept types, factories, and conversions are designed.
status: draft
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

These principles describe *how* to design a public Precept API, while
[API Admission Rules](api-admission-rules.md) describe *whether* an API is accepted at all.

Most of them apply to every public API. Where a principle holds only for a subset of the
vocabulary — the view types, for instance — it states that scope itself. A principle written for
one family is not a constraint on families that do not exist yet.

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
wrapper. Validate, then hand the standard type back: a runtime-sized span checked to hold exactly
`N` elements is a fixed-extent `std::span<T, N>`, and no Precept type is needed to carry that
fact any further.

New types are justified only when the invariant must survive beyond the check.

How a *failed* validation reaches the caller is a different question. The v0.1 span family uses
the boundary recorded in
[ADR-0006](../decisions/adr-0006-validation-and-conversion-boundaries.md).

# Constraint to better API

A constraint should improve how callers write code, not merely record a fact. `block_span`
exists because it lets callers iterate fixed-size blocks without tail handling — not
because `size() % N == 0` is worth recording on its own.

# Invariants must not be bypassable

If a type exists, its invariant holds. Every public construction path — constructors,
factories, conversions, assignment — must preserve it. An invariant that can be sidestepped
is worse than no invariant, because readers stop trusting the signature.

# Precept view types do not own storage

Precept's view types — the span family, and any later type that borrows a range it did not
allocate — are borrowed views. They do not own, allocate, or extend the lifetime of the underlying
storage, and must not appear to do so. Lifetime rules follow those of the standard view they wrap.

This is a principle about view types, not about Precept as a whole. Candidates that carry a value
rather than borrow a range — numeric refinements, transition types, deferred-initialization
types — are not views, and their ownership and lifetime semantics are not settled here.

# No magic

No hidden global state, no implicit registration, no required runtime, no code generation.
What the signature says is what happens.

# Better than an assert

An API earns its place only if it does more than move an `assert` somewhere else: the
verified fact must be visible in the signature and reusable downstream.

# v0.1 span contract

The validation failure model, unchecked-construction policy, `constexpr` and `noexcept` policy,
CTAD, conversions, and exact API behavior are defined by the
[v0.1 Span Family API Contract](../api/span-family.md). The expensive validation and conversion
boundary decisions are recorded in
[ADR-0006](../decisions/adr-0006-validation-and-conversion-boundaries.md).
