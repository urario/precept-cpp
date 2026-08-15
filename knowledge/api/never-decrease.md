---
type: API Contract
title: never_decrease API Contract
description: Defines the per-object non-decreasing value history, admitted integral types, transition failure, and usage boundary of never_decrease.
status: draft
sources:
  - id: issue-40
    resource: https://github.com/urario/precept-cpp/issues/40
    title: v0.2 semantic vocabulary investigation and DEFER implementation policy
    author: human:urario
  - id: issue-44
    resource: https://github.com/urario/precept-cpp/issues/44
    title: Investigation of whether a non-decreasing transition can be represented as a semantic type
    author: human:urario
  - id: issue-61
    resource: https://github.com/urario/precept-cpp/issues/61
    title: Implementation experiment and representative usage re-evaluation for never_decrease
    author: human:urario
tags: [api, transition, integral, experimental]
---

# Scope

`precept::never_decrease<T>` is a small, non-thread-safe value type for one transition rule:

```text
current -> next when next >= current
```

The type is an implementation experiment from issue #61. Its presence in the repository is
evidence gathering, not a stable-release promise.

# Admitted types and invariant

`T` must be an unqualified `std::integral` type other than `bool`. Floating-point types, `bool`,
and cv-qualified types are not admitted. The restriction keeps the ordering self-contained and
avoids NaN, signed-zero, and non-total-ordering questions.

Construction accepts any value of `T` and records it as the object's initial value. For the
remainder of that object's lifetime, its observable history through the public API is:

```text
v0 <= v1 <= v2 <= ...
```

The guarantee is per object. Separate objects have separate histories, and the type says nothing
about global, process-wide, distributed, cross-object, persistent, or wall-clock ordering.

# Public API

```cpp
template<std::integral T>
  requires (!std::same_as<T, bool> && std::same_as<T, std::remove_cv_t<T>>)
class never_decrease {
public:
    explicit constexpr never_decrease(T initial) noexcept;
    constexpr T value() const noexcept;
    constexpr bool try_update(T next) noexcept;
};
```

The actual declaration uses an internal concept for the constraint. There is no default
constructor, implicit conversion, assignment, arithmetic operator, comparison operator, reset,
callback, observer, custom error type, or decrease escape hatch.

`try_update(next)` returns `true` and stores `next` when `next >= current`. Equality is a valid
non-decreasing transition. It returns `false` when `next < current` and leaves the stored value
unchanged. Regression is the only failure reason, so `bool` is sufficient for this generic type.

# Copy, move, and assignment

Copy construction and move construction are available. The destination starts with the source's
current value and then has an independent history. For integral `T`, moving preserves the same
scalar value in the source and destination.

Copy assignment and move assignment are deleted. Assignment would replace an object's current
value without going through a non-decreasing transition; for example, assigning a value at `10`
from an object at `100` would make the observable history regress. Deleting both forms keeps the
history invariant mechanically enforced instead of giving assignment a special exception policy.

# Boundaries

The type is an ordinary value type and provides no synchronization or atomicity. Concurrent
access to one object requires caller-owned synchronization. It does not provide a process-wide
sequence, shared logical history, persistence, or external-state tracking.

The type validates a candidate transition only. It does not validate arithmetic used to produce
the candidate. Callers must avoid overflow before passing `next`; this type deliberately has no
increment or other arithmetic operation.

The stored integral value is self-contained, so its comparison does not become stale through an
external alias. This admission condition is not extended to arbitrary comparable types.

# Representative usage evidence

## Progress / processed count: positive evidence

The runnable `examples/never_decrease_transitions.cpp` passes one carrier through primary-path and
replay-path reporting functions. Both update sites report the same authoritative cumulative count,
and a regression is propagated as an invalid report: the coordinator stops before forwarding the
misleading value downstream. The type keeps the transition contract at the reporting API boundary
instead of relying on each updater to remember a local setter check.

This is the clearest positive case found in the experiment: the rule is reused across multiple
update sites and the per-object history is the intended semantic scope.

## High-water mark: negative evidence for this abstraction

A high-water mark normally consumes a lower candidate by ignoring it:

```cpp
current = (std::max)(current, candidate);
```

`never_decrease<T>` instead reports a rejected transition. Both preserve a non-decreasing stored
value, but their failure behavior differs. The closed `std::max` operation is more direct when
the domain wants observation of the greatest value rather than rejection of a stale candidate.

## Sequence / revision / generation: domain-specific pressure

The example compares `never_decrease<std::uint64_t>` with a small configuration-revision object.
The generic type accepts an equal update, while the revision operation rejects duplicates and may
need additional conflict, skipped-value, or wraparound policy. A sequence, revision, or generation
should therefore use a domain-specific type when those policies are part of its contract.

## Local setter: negative evidence for a one-site rule

The example also implements the same comparison as a local raw-value setter. When no update rule
crosses an API boundary and there is only one owner, the local check is shorter and clearer than
introducing a carrier.

# Actual friction

The positive progress example returns the `bool` from both update sites to a coordinator, so a
regression becomes an explicit control-flow failure rather than an ignored observation. Downstream
code that needs a plain count also calls `.value()`. These are acceptable carrier boundaries in the
positive case, but they make the wrapper less attractive for one-site code.

The comparison example exposed a semantic mismatch rather than an API defect: a generic
non-decreasing transition cannot express revision duplicate rejection. Adding an error enum or a
policy framework would hide that mismatch, so neither is part of this type.

# Comparison with set_once

Both types model a per-object transition history and close public mutation paths that would bypass
the rule:

```text
set_once<T>:
    unset -> set

never_decrease<T>:
    value -> value' where value' >= value
```

`set_once<T>` starts unset and deletes assignment because replacement would violate its one-way
slot history. `never_decrease<T>` starts with an explicit scalar and deletes assignment because
replacement could regress the current value. Both allow copy/move construction to create an
independent object history and both deliberately make thread safety and contained-value semantics
separate concerns.

The comparison also shows the genericization boundary: `set_once` does not claim non-null,
conflict, or lifecycle semantics, and `never_decrease` does not claim progress, high-water,
sequence, revision, or generation semantics. The shared transition shape is useful design
evidence, not a reason to add a public transition framework.

# Admission judgment

**KEEP (use-boundary)** as an experimental vocabulary candidate:

* use it for a self-contained integral value whose non-decreasing transition itself is the reusable
  contract, especially progress or processed-count reporting across multiple update sites;
* use `std::max` for a high-water mark when lower candidates should be ignored;
* use a domain-specific sequence, revision, or generation type when duplicate, conflict, skipped,
  or wraparound semantics matter; and
* use a local setter when the rule is local and one-shot.

This judgment does not promote `never_decrease<T>` to a general monotonic framework or erase the
#44 conclusion that `next >= current` is not the whole domain semantic. It records the new evidence
that the narrow progress-shaped case can carry an independent transition vocabulary.
