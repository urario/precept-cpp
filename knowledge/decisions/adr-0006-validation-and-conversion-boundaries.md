---
type: Architecture Decision
title: Define validation and conversion boundaries for the v0.1 span family
description: Use optional runtime validation, reject invariant bypasses, and allow truthful standard-range weakening.
status: stable
verified:
  - by: human:urario
    at: 2026-08-14T17:36:23+09:00
sources:
  - id: issue-4
    resource: https://github.com/urario/precept-cpp/issues/4
    title: Design issue for the v0.1 span API contract, failure model, and conversions
    author: human:urario
  - id: issue-4-final-review
    resource: https://github.com/urario/precept-cpp/issues/4#issuecomment-5287680856
    title: Final review resolution for validation and conversion boundaries
    author: human:urario
  - id: issue-20-resolution
    resource: https://github.com/urario/precept-cpp/issues/20#issuecomment-5288715908
    title: Resolution distinguishing safe semantic weakening from invariant bypass
    author: human:urario
tags: [architecture, api-design, span, validation, conversion]
---

# Context

The v0.1 span family must turn size preconditions into facts that callers and downstream APIs can
trust. Its construction boundary is expensive to change after release: a public unchecked path,
implicit loss of a fact, or a failure model becomes part of every caller's control flow.

The family also has both compile-time and runtime size information. Treating a contradiction that
is already visible in a fixed extent as a runtime failure would discard a semantic fact the type
system already carries.

# Decision

Runtime size validation returns `std::optional`. An ordinary mismatch returns `std::nullopt` and
does not assert, throw, terminate, or construct an invalid object.

There is no public unchecked construction path. Constructors, factories, conversions, and
assignment must all preserve their target invariant.

An input whose fixed extent makes the requested invariant impossible is rejected at compile time.
Runtime validation is reserved for a dynamic extent, except that `checked_span` also accepts a
fixed extent equal to its requested exact size while keeping its optional return type.

The v0.1 validation boundary accepts `std::span` only. Containers, ranges, pointer/count pairs, and
iterator pairs first cross an explicit standard-library boundary chosen by the caller.

Precept does not add an implicit conversion operator to `std::span`. Standard-library conversions
that arise naturally from truthful range modeling are allowed when they only weaken guarantees and
cannot create an invalid target state. `at_least_span` therefore remains a contiguous, sized, and
borrowed range even though the standard `std::span` range constructor can use those properties for
implicit conversion to a compatible dynamic-extent span.

Safe semantic weakening is distinct from invariant bypass. It is implicit only when the source
contract logically satisfies the target contract without runtime validation, preserves element
qualification and ownership/lifetime semantics, and creates no false target guarantee. Unsafe or
strengthening conversion must not bypass validation.

`as_span()` is the explicit named observer for obtaining the underlying dynamic-extent standard
span. It remains useful when callers want that boundary visible, but it is not the only permitted
weakening path.

# Alternatives considered

Issue #4 considered exceptions, assertions, termination, error enums, and bool/out-parameter
validation. These were rejected in favor of `std::optional` because a size mismatch is an ordinary,
single-reason validation failure.

Issue #4 also considered a public unchecked or assume-valid construction path. It was rejected
because it would make a semantic type's signature unreliable.

Generic contiguous, sized, borrowed-range validation overloads were considered and rejected for
v0.1. They add range lifetime, type-conversion, overload-resolution, and exception-specification
complexity outside the span precondition being tested. Callers can construct `std::span` first.

Handling a statically impossible fixed extent as `std::nullopt` was considered and rejected. The
fixed extent already proves that the request cannot succeed.

Issue #4 rejected adding an implicit Precept conversion operator to `std::span`; that rejection is
retained. Issue #20 found that `std::span` can nevertheless construct from `at_least_span` through
the standard range interface. Removing borrowed, contiguous, or sized range modeling was rejected
because it either fails to block lvalue conversion or misrepresents the view's actual properties.
A deleted conversion operator was also rejected because the conversion originates in the target
standard type and the deletion does not block it.

# Consequences

Callers have one non-throwing runtime failure model and cannot bypass a wrapper invariant through a
Precept API. Compile-time facts remain compile-time constraints.

Containers and other ranges still require an explicit `std::span{range}` step before validation,
marking where lifetime responsibility enters Precept. Callers may use `as_span()` to make semantic
weakening visible, while compatible standard-library weakening from `at_least_span` may remain
implicit.

Future generic-range validation overloads require separate evidence and design review; this
decision does not reserve them or add a speculative abstraction for them.

Detailed signatures, individual operations, zero-size behavior, and iterator semantics belong to
the [v0.1 Span Family API Contract](../api/span-family.md), not this ADR.

# Status

Accepted.

# Related

* Issue [#4](https://github.com/urario/precept-cpp/issues/4) — v0.1 span API contract
* Issue [#20](https://github.com/urario/precept-cpp/issues/20) — truthful range interoperability
  and semantic weakening
* [Design Principles](../architecture/design-principles.md)
* [API Admission Rules](../architecture/api-admission-rules.md)
* [v0.1 Span Family API Contract](../api/span-family.md)
