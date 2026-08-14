---
type: Design Principle
title: Precept Design Principles
description: The principles that shape how Precept semantic types, factories, operations, and proof carriers are designed.
status: stable
verified:
  - by: human:urario
    at: 2026-08-14T21:47:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
  - id: issue-4
    resource: https://github.com/urario/precept-cpp/issues/4
    title: Design issue for the v0.1 span API contract, failure model, and conversions
    author: human:urario
  - id: issue-49-owner-decision
    resource: https://github.com/urario/precept-cpp/issues/49#issuecomment-5293470398
    title: Owner approval of the v0.2 API admission and design-principle baseline
    author: chatgpt/gpt-5.6-sol
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

The value of Precept is not a shorter guard clause. It is that a semantic fact, verified once, is
preserved by the chosen representation and remains useful across API boundaries.

That representation may be a semantic type, a relational proof carrier, a transition-restricted
API, or a standard type when the fact is fully expressible there. The representation should match
the fact instead of forcing every fact into a wrapper type.

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

New types are justified only when the invariant must survive beyond the check and no standard type
already carries it completely.

How a *failed* validation reaches the caller is a different question. The v0.1 span family uses
the boundary recorded in
[ADR-0006](../decisions/adr-0006-validation-and-conversion-boundaries.md).

# Constraint to better API

A constraint should improve how callers write code, not merely record a fact. `block_span`
exists because it lets callers iterate fixed-size blocks without tail handling — not
because `size() % N == 0` is worth recording on its own.

# Invariants must not be bypassable

Every public API path that claims a semantic fact must preserve that claim. Construction,
validation, conversion, update, relation-producing operations, and observation must not create a
state in which the API surface says a fact holds when it does not.

A semantic representation that can be bypassed is worse than no representation, because readers
stop trusting the signature and returned proof objects.

# Proof freshness must be explicit

A proof is useful only while the fact it represents remains trustworthy. Every persistent semantic
representation must make clear what mutation, aliasing, lifetime change, external state, copy, or
move can invalidate the fact.

If the fact can become stale while the Precept representation still appears valid, and that
freshness boundary cannot be stated as a small, readable contract, the fact should be reshaped or
rejected as a persistent Precept proof.

# Do not propagate guarantees across non-closed operations

A semantic guarantee is propagated through an operation only when that operation is closed over
the invariant. If the result does not necessarily satisfy the same guarantee, do not return the
stronger semantic type or proof merely because the inputs had it.

Prefer the appropriate standard result type, or require validation again when the fact must be
re-established. This keeps semantic vocabulary from growing into a speculative operator or policy
framework.

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

An API earns its place only if it does more than move an `assert` or guard clause somewhere else.
The verified fact must remain useful after validation through the chosen representation, including
cases where a standard return type fully embodies that fact.

# v0.1 span contract

The validation failure model, unchecked-construction policy, `constexpr` and `noexcept` policy,
CTAD, conversions, and exact API behavior are defined by the
[v0.1 Span Family API Contract](../api/span-family.md). The expensive validation and conversion
boundary decisions are recorded in
[ADR-0006](../decisions/adr-0006-validation-and-conversion-boundaries.md).
