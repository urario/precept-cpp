---
type: Architecture Decision
title: Prefer a dedicated semantic vocabulary over a generic constrained framework
description: Provide named types such as at_least_span instead of a general predicate-parameterized refinement framework.
status: draft
generated:
  by: claude-code
  at: 2026-08-13T00:00:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
  - id: issue-4
    resource: https://github.com/urario/precept-cpp/issues/4
    title: Design issue for the v0.1 span API contract, failure model, and conversions
    author: human:urario
tags: [architecture, api-design, vocabulary]
---

# Context

Once a library can attach one precondition to a type, generalizing is tempting: a single
refinement template parameterized by arbitrary predicates covers every case at once.

```cpp
refined_span<const std::byte, size_at_least<16>, size_multiple_of<8>>
```

Precept's stated purpose is that a precondition is readable directly from the signature —
by humans, and by AI agents generating or modifying code around it. A predicate DSL moves
the meaning from the name into a composition that has to be decoded, and its diagnostics
degrade quickly.

# Decision

Precept provides dedicated, self-explanatory types and operations for each semantic rule —
`at_least_span`, `non_empty_span`, `block_span`, `checked_span()` — rather than a generic
constraint or predicate framework. Each name states one rule.

Generality is not a goal in itself. A new rule earns a new name only when it passes the
[API Admission Rules](/architecture/api-admission-rules.md).

# Alternatives considered

* **Generic predicate framework** — maximum expressiveness and no per-rule code, but
  signatures stop being self-documenting, error messages become template noise, and the
  library turns into a DSL to learn rather than a vocabulary to read. Rejected.
* **Dedicated types implemented privately on top of a generic core** — keeps the readable
  public names while sharing implementation. Not adopted now: with the v0.1 surface this
  small, the shared core would be speculative, and it risks leaking into public
  signatures and diagnostics. It stays available as a purely internal refactoring later.
* **Only free validation functions, no types at all** — simple, but the verified fact is
  lost immediately after the call, which defeats the purpose of carrying it across API
  boundaries. Rejected.

# Consequences

* The public surface grows one deliberate name at a time, and each addition is a design
  decision rather than a template instantiation.
* Rules that nobody has asked for do not exist, keeping the library small.
* Some duplication between related types is accepted in exchange for readable signatures
  and diagnostics.
* Users needing an unsupported precondition cannot compose one from Precept parts; they
  request it, and it is evaluated against the admission rules.
* Naming quality becomes a first-class concern, since the name is the documentation.

# Status

Accepted.

# Related

* Issue [#1](https://github.com/urario/precept-cpp/issues/1) — roadmap and base policy
* Issue [#4](https://github.com/urario/precept-cpp/issues/4) — v0.1 span API contract
* [Design Principles](/architecture/design-principles.md)
* [API Admission Rules](/architecture/api-admission-rules.md)
