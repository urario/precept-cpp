---
type: API Contract
title: non-overlapping spans API contract
description: Defines the byte-range relation, validation, observation, and borrowed lifetime boundary of non_overlapping_spans.
status: draft
sources:
  - id: issue-48
    resource: https://github.com/urario/precept-cpp/issues/48
    title: Experiment for a portable non-overlapping contiguous-range relation
    author: human:urario
  - id: cpp20-draft
    resource: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf
    title: C++20 working draft N4861
    author: organization:wg21
tags: [api, relation, span, lifetime, v0.2]
---

# Scope

The initial API accepts two `std::span` values. It does not accept containers, iterator pairs,
pointer-and-count pairs, segmented ranges, or multidimensional views. Volatile element types are
excluded because C++20 `std::as_bytes` does not preserve volatile qualification.

The relation concerns storage only. It says nothing about element values, ownership, semantic
correspondence, synchronization, or whether either range is otherwise valid for a particular
operation.

# Portable contract

`non_overlapping_spans<T, U>` stores copies of two spans whose object-representation byte ranges
share no byte.

* An empty span overlaps nothing, regardless of its `data()` value.
* Adjacent half-open ranges such as `[0, 8)` and `[8, 16)` do not overlap.
* Subspans of one backing array are judged by the bytes they cover, so overlapping, touching, and
  separated subspans have the expected results.
* Spans over unrelated live objects are valid inputs.
* Different element types are compared by byte extent, not by element count.

A short user-facing statement is therefore:

> On success, the returned object stores the two supplied span snapshots, and their
> object-representation byte ranges share no byte. Empty ranges share no byte with any range, and
> ranges that only touch at an endpoint are accepted.

# Validation and portability

`checked_non_overlapping(first, second)` returns an engaged optional when the relation holds and
`std::nullopt` otherwise. It neither truncates a range nor reads or modifies element values.

The implementation deliberately does not apply built-in relational operators to unrelated
pointers, convert pointers to integers, or interpret the implementation-defined total order from
`std::less` as a linear address space. C++20 guarantees a total order for pointer comparison
function objects, but that alone does not state that byte intervals belonging to unrelated objects
cannot interleave in that order.

Instead, validation forms `std::as_bytes` views. For two non-empty contiguous byte intervals,
overlap exists exactly when the first byte of either interval is contained in the other. The
implementation searches both byte views using pointer equality and never compares an unrelated
pointer with a one-past pointer. Empty ranges return before any such search.

This portable route is linear in `first.size_bytes() + second.size_bytes()`. The API does not claim
constant-time validation or `constexpr` validation in C++20.

# Carrier shape and observation

The successful value stores the original typed spans, not byte views. `first()` and `second()`
return those span snapshots by value, preserving element type and const qualification for ordinary
C++ operations. Returning by value prevents a caller from retargeting the spans stored inside the
carrier.

Copy and move preserve the snapshots and the relation. Mutating elements through a mutable stored
span cannot change addresses or extents and therefore cannot change the relation. Reassigning one
of the source span variables after validation also has no effect on the stored snapshots.

# Freshness and lifetime

The carrier is a borrowed view and does not own, allocate, or extend the lifetime of storage. Its
lifetime rule is the ordinary `std::span` rule: destruction or reallocation of an owner, the end of
an element's lifetime, or storage reuse that invalidates either stored span also makes using the
carrier invalid. The carrier does not claim that a current container or a later span still denotes
the validated ranges.

No ownership model, borrow token, registry, or lifetime tracking is needed to state this boundary.

# Representative usage result

The runnable buffer example exercised three different call shapes.

| Usage | Relation lifetime | Reuse | Result |
|---|---:|---:|---|
| one-shot copy | zero public hops; validated and unwrapped inside one operation | once | operation-owned validation is clearer; carrier is negative evidence here |
| three-stage buffer processing | three function calls | three times | carrier keeps the non-overlap precondition in every stage signature and amortizes linear validation |
| input/output/scratch processing | three phase calls | pair facts are not passed separately | a role-bearing domain aggregate is clearer than three pair carriers |

The carrier is therefore not the default shape for every non-overlap check. It is useful only when
the same pair relation itself survives across an API boundary and is consumed more than once.

# Comparison with same-size

Issue #46 found that equal-size relation carriers were mechanically possible but usually lost to a
local operation. Both relations can store span snapshots and state borrowed lifetime rules.

The distinguishing evidence here is that portable non-overlap validation is linear in byte extent,
while size equality is constant time. Reusing a non-overlap carrier can therefore avoid repeated
work as well as repeated guard clauses. The multi-stage usage also consumes exactly the generic
pair relation, whereas scratch-buffer usage confirms the #46 warning that a richer domain relation
should win when the generic fact is only a necessary fragment.

# Friction observed during implementation

1. A portable constant-time address-interval check could not be justified from the standard
   pointer total order alone; the equality-only implementation is linear in bytes.
2. The one-shot copy immediately called `first()` and `second()`, so exposing the carrier to its
   caller added no value.
3. Pairwise scratch validation produced three temporary carriers and repeated each buffer role;
   downstream code became clearer with a domain-specific three-buffer aggregate.
4. A bare `non_overlapping(...)` factory looked like a Boolean predicate, so the implemented
   spelling uses `checked_non_overlapping(...)` for an optional carrier result.
5. Volatile spans had to be excluded at the `std::as_bytes` boundary.

# Recommendation

**KEEP, with a use boundary:** keep the relation carrier for a validated pair that crosses an API
boundary and is reused. Prefer an operation-owned check for one-shot work, and prefer a
domain-specific aggregate when roles or three-way relations are the real contract. Do not grow a
generic relation framework or a memory-region algebra from this API.
