---
type: Project Rule
title: API Admission Rules
description: The eight rules a proposed public Precept API must satisfy before it is accepted.
status: stable
verified:
  - by: human:urario
    at: 2026-08-14T21:47:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
  - id: issue-49-owner-decision
    resource: https://github.com/urario/precept-cpp/issues/49#issuecomment-5293470398
    title: Owner approval of the v0.2 API admission and design-principle baseline
    author: chatgpt/gpt-5.6-sol
tags: [architecture, api-design, rules]
---

# How to use these rules

A proposed public API must satisfy **all eight** rules. Failing one is not a detail to fix
later; it is a reason to reshape or reject the proposal. Use this list when opening an API
proposal issue and when reviewing a pull request that adds or changes public API.

# 1. One rule

An API expresses one kind of semantic rule. `at_least_span` states a minimum size and
nothing else. Combining unrelated guarantees into a single type makes the name unreadable
and the invariant hard to verify.

# 2. Readable

The name carries most of the meaning. A reader who has never used Precept should guess the
guarantee correctly from the signature alone.

# 3. Enforced

The guarantee holds mechanically through the public representation and its validated boundaries —
for example a semantic type, a transition API, a relational proof carrier, or a validating
factory — not through comments, naming conventions, or reviewer vigilance.

# 4. Reusable

The rule is general enough to appear in unrelated codebases. Product-specific or
domain-specific policies do not belong in Precept.

# 5. Tiny

No large framework, runtime, build-time code generation, or heavyweight compile-time
machinery. Adoption cost stays close to including a header.

# 6. Composable

The API integrates naturally with standard C++ types and operations. It builds on the standard
library rather than displacing it, and returns to a standard type whenever the verified fact is
fully expressible there.

What "natural" means differs per family. A view should interoperate truthfully with the relevant
range and algorithm facilities; a transition type should expose only the state changes its
contract permits; a relational proof should compose with ordinary operations without requiring a
relation framework; and a validating factory should return the standard type when that type fully
carries the verified fact.

Composable does **not** mean reproducing every operation of an underlying type. An operation that
is not closed over the semantic guarantee need not preserve or re-create that guarantee.

# 7. No magic

No hidden global state, no implicit registration, no surprising control flow. Behavior is
predictable from the signature.

# 8. Better than an assert

The API must deliver more than relocating an assertion or shortening a guard clause. The verified
fact must remain useful after validation — for example by being visible in a semantic type or
proof carrier, by constraining allowed transitions, or by being fully embodied in a returned
standard type that downstream code can use without repeating the original check.

# Relationship to design principles

These rules are the admission gate. [Design Principles](design-principles.md)
describe how an admitted API is then shaped.
