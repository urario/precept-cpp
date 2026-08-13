---
type: Project Charter
title: Precept Project Charter
description: Precept makes semantic preconditions of C++ APIs explicit, reusable, and machine-enforced.
status: draft
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
  - id: issue-4
    resource: https://github.com/urario/precept-cpp/issues/4
    title: Design issue defining the v0.1 span API contract
    author: human:urario
tags: [vision, charter, scope]
---

# Purpose

Precept is a header-only C++ library that turns semantic preconditions and invariants —
usually buried inside function bodies — into small, readable types and operations that can
be stated once, carried with the value, and reused across API boundaries.

# Problem

A typical C++ signature hides its most important design condition:

```cpp
void parse(std::span<const std::byte> packet)
{
    if (packet.size() < 16) return;
    // ...
}
```

`size() >= 16` is a design condition, but it is invisible in the signature. Every
downstream function must re-check it, comment it, or trust human memory.

Precept states the condition where callers can see it:

```cpp
void parse(precept::at_least_span<const std::byte, 16> packet);
```

A fact verified once travels with the value and is reusable at deeper layers.

# Why build this after generative AI

Generating guard clauses and boilerplate is cheap now, so saving a few lines is not the
point. Precept optimizes for a different set of properties:

* Design assumptions stay visible in the API signature.
* Implementations and edits — human or AI-generated — are less likely to silently drop a
  precondition.
* A fact the caller already validated is reusable in deeper layers.
* Conditions are enforced mechanically by types and operations, not by comments.
* The vocabulary reads clearly for both humans and AI agents.

```text
implicit assumption
    -> explicit semantic vocabulary
    -> reusable + enforceable API contract
```

# Base policy

* Minimum language version: C++20 — see [ADR-0001](../decisions/adr-0001-cpp20.md).
* Form: header-only, zero consumer dependencies — see [ADR-0002](../decisions/adr-0002-header-only.md).
* Public namespace: `precept`.
* License: Apache-2.0 — see [ADR-0003](../decisions/adr-0003-apache-2-0.md).
* Knowledge format: Open Knowledge Format (OKF) v0.2, rooted at [this bundle](../index.md).
* Build thinly on top of the standard library; never replace it.
* Prefer a small, self-explanatory vocabulary over a generic constraint framework — see
  [ADR-0004](../decisions/adr-0004-dedicated-semantic-vocabulary.md).

The design principles behind these choices are recorded in
[Design Principles](../architecture/design-principles.md), and the gate every public API
must pass is [API Admission Rules](../architecture/api-admission-rules.md).

# v0.1 scope

v0.1 is limited to size preconditions on spans — see
[ADR-0005](../decisions/adr-0005-v0-1-span-scope.md):

```cpp
precept::at_least_span
precept::non_empty_span
precept::block_span
precept::checked_span(...)
```

The hypothesis being tested:

> C++ developers want to state frequently recurring `std::span` size preconditions as
> parameter types or validated factories, and reuse them.

Their exact contracts, conversions, and failure model are defined by the
[v0.1 Span Family API Contract](../api/span-family.md). They remain outside this charter's
responsibility.

# Explored after v0.1

Candidates only — not commitments:

* Refined arguments: `aligned_ptr`, `finite`, `nonzero`, `power_of_two`
* Transition semantics: `never_decrease`, `never_increase`, `late`
* Semantic operations: `parse_exact()`, `zip_exact()`, `read_exact()`, `write_all()`
* Runtime protocol guards: `no_overlap`, `must_complete`, `latest::gate`

Whether the runtime protocol family belongs in Precept at all is re-evaluated later.

# Success criteria

Star count is not a success criterion. Precept succeeds when:

* It is usable in real code.
* The wrappers do not obstruct ordinary C++ operations.
* Preconditions are readable from the signature.
* Downstream re-checking decreases.
* Users naturally ask for other preconditions expressed the same way.
* C++20, header-only, and zero consumer dependencies are preserved.
* Humans and agents can trace design decisions and rules from this knowledge bundle.

# Non-goals

* Becoming a general-purpose constraint or predicate framework.
* Replacing or fully wrapping standard library types.
* Owning storage.
* Providing a C++17 compatibility span.
* Growing into a grab bag of convenience APIs.
