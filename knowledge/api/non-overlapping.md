---
type: API Contract
title: non-overlapping spans API contract
description: Defines the portable storage non-overlap relation for byte-sized spans, validation, observation, and borrowed lifetime boundary.
status: draft
sources:
  - id: issue-48
    resource: https://github.com/urario/precept-cpp/issues/48
    title: Experiment for a reusable non-overlapping contiguous-range relation
    author: human:urario
  - id: issue-66
    resource: https://github.com/urario/precept-cpp/issues/66
    title: C++20 portability review for non_overlapping_spans
    author: human:urario
  - id: issue-67
    resource: https://github.com/urario/precept-cpp/issues/67
    title: Portable byte-sized span hardening
    author: human:urario
  - id: issue-65-final-admission
    resource: https://github.com/urario/precept-cpp/issues/65#issuecomment-5301911732
    title: Final v0.2 admission matrix establishing the 8/8 stable portfolio
    author: chatgpt/gpt-5.6-sol
  - id: p1839r7
    resource: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p1839r7.html
    title: Accessing object representations
    author: organization:wg21
    optional: true
tags: [api, relation, span, lifetime, portability, v0.2]
---

# Scope

The stable v0.2 API accepts two `std::span` values only when each element type occupies exactly
one byte:

```text
sizeof(T) == 1
sizeof(U) == 1
```

This includes ordinary byte-buffer element types such as `std::byte`, `char`, and
`unsigned char`, with cv-qualification preserved. It deliberately excludes wider typed spans such
as `std::span<int>` or `std::span<float>`.

The public header is `<precept/non_overlapping.hpp>`.

# Exact relation

A successful carrier guarantees that the storage occupied by the two stored span snapshots shares
no byte.

* An empty span overlaps nothing.
* Two ranges that only touch at an endpoint do not overlap.
* Overlapping subspans of one byte buffer are rejected.
* Disjoint subspans of one byte buffer are accepted.
* Different byte-sized element types are admitted when their spans are otherwise valid.

The relation concerns storage only. It says nothing about values, ownership, semantic
correspondence, synchronization, or whether either range satisfies additional requirements of a
particular operation.

# Why the element-size boundary exists

The original implementation accepted arbitrary typed spans and converted them with
`std::as_bytes` before comparing object-representation byte addresses. Issue #66 re-examined that
basis for stable admission.

P1839R7 documents a defect in the current C++ object-representation wording: direct traversal of
an object's representation through char-like pointers is widely intended and implemented, but the
current specification does not make the general technique well-defined. The paper specifically
identifies `std::as_bytes` as a standard-library facility whose intended implementation depends on
that missing language support.

Precept therefore does not make its v0.2 stable contract depend on traversing an arbitrary typed
object representation.

For a span whose element type has `sizeof(T) == 1`, no object-representation view is needed. The
span already denotes the actual contiguous sequence of one-byte element objects. Its occupied
storage is exactly the set of addresses represented by those elements.

This narrowed scope turns the portability issue into an ordinary span-and-pointer-equality problem
and keeps the useful byte-buffer use case.

# Validation

`checked_non_overlapping(first, second)` returns an engaged optional when the relation holds and
`std::nullopt` when at least one byte of storage is shared.

For two non-empty contiguous ranges of one-byte elements, overlap exists exactly when the first
element address of either range occurs among the element addresses of the other range. Validation
therefore scans both spans using pointer equality only.

The implementation does not:

* call `std::as_bytes` or `std::as_writable_bytes`;
* order unrelated pointers with built-in relational operators;
* interpret `std::less` as a linear address space; or
* convert pointers to integers.

Validation is linear in the sum of the two element counts and does not read or modify element
values.

# Carrier shape and observation

The successful value stores the original typed spans, not an erased byte view.

```cpp
template<class T, class U>
  requires (sizeof(T) == 1 && sizeof(U) == 1)
class non_overlapping_spans;
```

`first()` and `second()` return the stored span snapshots by value, preserving element type and
cv-qualification. Returning by value prevents callers from retargeting the spans stored inside the
carrier.

Copy and move preserve the snapshots and the relation. Mutating an element through a mutable span
cannot move or resize the stored ranges and therefore cannot change the non-overlap relation.

# Freshness and lifetime

The carrier is a borrowed view and does not own, allocate, or extend storage lifetime. Destruction
or reallocation of an owner, end of an element lifetime, or storage reuse that invalidates either
stored span also invalidates use of the carrier.

Reassigning one of the source `std::span` variables after validation does not retarget the stored
snapshots.

No ownership model, borrow token, registry, or lifetime tracking is part of this API.

# Representative usage

The runnable `examples/non_overlapping_buffers.cpp` uses `std::byte` buffers and exercises three
shapes.

| Usage | Relation lifetime | Result |
| --- | --- | --- |
| one-shot copy | validated and consumed inside one operation | operation-owned validation is clearer |
| multi-stage input/output pair | same validated pair crosses multiple consumers | carrier keeps the relation visible in signatures |
| input/output/scratch roles | true contract is a role-bearing three-buffer set | domain-specific aggregate is clearer than three pair carriers |

The carrier is therefore not the default shape for every overlap test. It is useful when the same
pair relation itself survives across an API boundary and is reused.

# Relationship to typed buffers

A caller with `std::span<float>`, `std::span<int>`, or another wider element type does not get a
stable v0.2 `checked_non_overlapping` overload for that typed span directly.

The library deliberately does not ask callers to use `std::as_bytes` merely to re-enter this API,
because that would recreate the portability basis removed by #66. A future language defect
resolution or a separately justified typed-buffer design may reopen that scope.

This is a scope reduction, not a rejection of the relation carrier itself.

# Public API sketch

```cpp
namespace precept {

template<class T, class U>
  requires (sizeof(T) == 1 && sizeof(U) == 1)
class non_overlapping_spans {
public:
    using first_element_type = T;
    using second_element_type = U;

    [[nodiscard]] constexpr std::span<T> first() const noexcept;
    [[nodiscard]] constexpr std::span<U> second() const noexcept;
};

template<class T, std::size_t E, class U, std::size_t F>
  requires (sizeof(T) == 1 && sizeof(U) == 1)
[[nodiscard]]
std::optional<non_overlapping_spans<T, U>>
checked_non_overlapping(std::span<T, E> first,
                        std::span<U, F> second) noexcept;

} // namespace precept
```

# Admission finding

**STABLE (use-boundary)** in the v0.2 public vocabulary:

```text
one-shot byte-buffer relation  -> operation-owned check
reused byte-buffer pair        -> non_overlapping_spans
role-bearing / N-way relation  -> domain-specific aggregate
wider arbitrary typed spans    -> outside v0.2 stable scope
```

The #66 / #67 portability hardening removed the only material blocker identified by the portfolio
review. Stable admission is intentionally limited to byte-sized element spans, where the relation
can be stated and implemented directly in C++20 without object-representation traversal.

# Required verification

Stable verification covers:

* disjoint one-byte spans;
* overlapping, touching, and separated subspans of the same storage;
* all empty-range combinations;
* cv-qualified byte-sized element types;
* different byte-sized element types for disjoint ranges;
* rejection of wider element types at compile time;
* copy and move preservation of stored snapshots;
* source-span reassignment not retargeting the carrier;
* absence of unchecked public construction;
* installed consumer use; and
* the runnable multi-stage buffer example.
