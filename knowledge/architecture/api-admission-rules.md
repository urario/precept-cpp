---
type: Project Rule
title: API Admission Rules
description: The eight rules a proposed public Precept API must satisfy before it is accepted.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-13T00:00:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
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

The guarantee holds mechanically through the type system and validated construction, not
through comments, naming conventions, or reviewer vigilance.

# 4. Reusable

The rule is general enough to appear in unrelated codebases. Product-specific or
domain-specific policies do not belong in Precept.

# 5. Tiny

No large framework, runtime, build-time code generation, or heavyweight compile-time
machinery. Adoption cost stays close to including a header.

# 6. Composable

The API connects naturally to the standard library: it accepts standard containers and
views as input, and returns to standard types when the fact is fully expressible there.

# 7. No magic

No hidden global state, no implicit registration, no surprising control flow. Behavior is
predictable from the signature.

# 8. Better than an assert

The API must deliver more than relocating an assertion: the verified fact becomes visible
in the signature and reusable by downstream callers.

# Relationship to design principles

These rules are the admission gate. [Design Principles](design-principles.md)
describe how an admitted API is then shaped.
