---
type: API Contract
title: same-size span relation API contract
description: Defines the stable dynamic-span carrier for equal cardinality and its semantic usage boundary.
status: draft
sources:
  - id: issue-62
    resource: https://github.com/urario/precept-cpp/issues/62
    title: Implementation experiment for the same-size relational proof
    author: human:urario
  - id: issue-46
    resource: https://github.com/urario/precept-cpp/issues/46
    title: Anchor investigation for reusable same-size relational proofs
    author: human:urario
  - id: issue-40
    resource: https://github.com/urario/precept-cpp/issues/40
    title: Common candidate evaluation protocol and relation-family evidence
    author: human:urario
  - id: issue-48
    resource: https://github.com/urario/precept-cpp/issues/48
    title: Non-overlapping span relation experiment
    author: human:urario
  - id: issue-65-final-admission
    resource: https://github.com/urario/precept-cpp/issues/65#issuecomment-5301911732
    title: Final v0.2 admission matrix establishing the 8/8 stable portfolio
    author: chatgpt/gpt-5.6-sol
tags: [api, relation, span, lifetime, v0.2]
---

# Scope

This stable v0.2 API surface represents exactly one relation between two dynamic-extent
`std::span` values. It does not accept arbitrary containers or fixed-extent spans. Fixed-extent
equality should remain expressed by the standard `std::span<T, N>` type, including after a
fixed-size validation of a dynamic span.

The public header is `<precept/same_size.hpp>`.

# API

```cpp
template <class T, class U>
class same_size_pair {
public:
  [[nodiscard]] constexpr std::span<T> first() const noexcept;
  [[nodiscard]] constexpr std::span<U> second() const noexcept;
  [[nodiscard]] constexpr std::size_t size() const noexcept;
};

template <class T, class U, std::size_t E, std::size_t F>
  requires(E == std::dynamic_extent && F == std::dynamic_extent)
[[nodiscard]] constexpr std::optional<same_size_pair<T, U>>
checked_same_size(std::span<T, E> first, std::span<U, F> second) noexcept;
```

`same_size_pair` has no public unchecked constructor. The only raw-span construction boundary is
`checked_same_size`. Unequal sizes return `std::nullopt`; equal sizes succeed, including empty /
empty. Empty / non-empty returns `std::nullopt`.

# Invariant

The exact invariant is:

```text
first().size() == second().size()
```

This is the only relation proved. Equal size does not imply element correspondence. The carrier
does not prove that `first[i]` corresponds semantically to `second[i]`, that the spans share a
domain or owner, or that they share a lifetime. It also says nothing about non-overlap, alignment,
sorting, uniqueness, or element values.

# Snapshot and lifetime semantics

The carrier stores copies of the two supplied `std::span` values. Reassigning either source span
variable after validation does not retarget the carrier. Copy and move preserve both snapshots.
The carrier is non-owning: the caller remains responsible for the lifetime and validity of the
underlying storage. Owner destruction, reallocation, end of element lifetime, or storage reuse
that invalidates a stored span also invalidates use of the carrier. The carrier does not track a
container's later logical size or provide a borrow/provenance framework.

Mutable and const element qualifications are preserved by the two observers. A mutable first
span remains mutable; `same_size_pair<int, const float>` is valid. The relation itself does not
add or remove element mutability.

# Usage evidence

The runnable [same-size relation example](../../examples/same_size_relations.cpp) compares four
shapes.

## One-shot

The local `lhs.size() != rhs.size()` check is clearer when validation is immediately followed by
one dot-product operation. A carrier version was also written for comparison, but it is validated
and unwrapped in the same function. It adds `.first()`, `.second()`, and optional-carrier ceremony
without carrying a fact across a boundary. This is negative evidence against using the carrier as
the default one-shot shape.

## Multi-stage

The example validates once at an input boundary, then passes `same_size_pair<float, const float>`
to `normalize` and `emit`, which both consume the paired buffers. `transform` deliberately accepts
the weakened `std::span<float>` returned by `first()` because it does not use the second span. The
carrier therefore survives two independent downstream consumers without pretending that every
stage needs the relation. This is the positive case: the value is not just a shortened guard
clause, and the caller/reviewer convention is replaced by a visible contract where the relation is
actually needed.

The carrier remains a structural relation only. If a pipeline needs roles, ownership, or a stronger
domain relationship, the caller should introduce a domain-specific aggregate instead.

## Domain correspondence

The example uses jobs and results. `checked_same_size` accepts same-length sequences even when a
result's `job_id` does not match the job at the same index. A local `job_results` aggregate adds
that domain-specific correspondence check and names the roles. This demonstrates the boundary:
generic equal cardinality must not be presented as semantic pairing, and tags or domain identity
must not be added to `same_size_pair` to compensate.

## Three or more sequences

The example also uses values, weights, and a mask. The natural contract is
`A.size() == B.size() == C.size()`, so a pair carrier is narrow for this shape. The example keeps
the check local and does not introduce a triplet, nested carrier, variadic relation, or generic
framework. This is N-way pressure to record for a future decision, not scope for this API.

# Comparison with `non_overlapping_spans`

| Concern | `same_size_pair` | `non_overlapping_spans` |
| --- | --- | --- |
| Validation cost | O(1), one size comparison | Current implementation is non-trivial and linear in byte extents |
| Relation | Equal element cardinality | Disjoint storage for byte-sized spans |
| Signature value | Keeps equal cardinality visible across reused stages | Keeps storage relation visible across reused stages |
| Reuse value | Demonstrated for a multi-stage paired-buffer pipeline; weak for one-shot work | Stronger when repeated validation would repeat a linear scan |
| Lifetime burden | Ordinary borrowed `std::span` lifetime | Ordinary borrowed `std::span` lifetime, with a stronger storage relation |
| Semantic weakness | Equal size does not imply correspondence | Non-overlap does not name buffer roles or ownership |

The experiment therefore separates validation savings from contract value. A cheap relation can
still be worth carrying when the relation itself crosses multiple API boundaries, but the cheap
check makes one-shot wrapper ceremony especially hard to justify.

# Actual friction

The implementation experiment observed the following:

* One-shot use needed immediate `.first()` / `.second()` unwrapping, while the local check already
  expressed the operation's complete precondition.
* Multi-stage signatures became explicit for the paired consumers, but the independent transform
  stage is clearer after weakening to `std::span<float>`; paired consumers still use `.first()`,
  `.second()`, and `.size()` rather than a domain-named view.
* The generic carrier was intentionally too weak for job/result correspondence; the local domain
  aggregate was clearer once index identity mattered.
* Three parallel sequences were naturally expressed by two local equality checks, so this binary
  carrier was not extended to N-way form.

# Admission judgment

**STABLE (use-boundary)** in the v0.2 public vocabulary:

```text
one-shot          -> local check or consuming operation
multi-stage pair  -> same_size_pair
domain pairing    -> domain-specific aggregate
N-way relation    -> local check until a separate design decision exists
```

Stable admission does not turn equal cardinality into correspondence and does not commit Precept
to a generic relational-proof framework. The earlier #46 DEFER decision remains useful historical
evidence: a relation should not be wrapped merely because it can be. The admitted case is the
narrow one demonstrated by #62, where the relation remains useful across multiple downstream
signatures and outweighs the carrier ceremony.
