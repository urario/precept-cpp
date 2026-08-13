---
type: Project Rule
title: Documentation Rules
description: Language policy, the split between README and this knowledge bundle, and how knowledge concepts are authored.
status: draft
generated:
  by: claude-code
  at: 2026-08-13T00:00:00+09:00
sources:
  - id: issue-3
    resource: https://github.com/urario/precept-cpp/issues/3
    title: Foundation issue for contribution, documentation, and AI-assisted development rules
    author: human:urario
  - id: issue-10
    resource: https://github.com/urario/precept-cpp/issues/10
    title: Foundation issue for managing project knowledge, rules, and ADRs with OKF v0.2
    author: human:urario
tags: [rules, documentation, okf]
---

# Language policy

Issues, pull requests, reviews, and discussions are written primarily in **Japanese**.
English is very welcome; nobody is asked to switch.

Repository artifacts are written in **English**:

* README
* source code comments
* public API documentation and comments
* identifiers
* examples
* user-facing text
* every concept in this knowledge bundle, including ADRs

# Public API comments are written for users

Public API comments address library users, not future maintainers of the implementation.
Where relevant they explain:

* the guarantee the type or function provides
* failure behavior
* lifetime and ownership (Precept views never own storage)
* the relationship to the corresponding standard library type

Implementation notes belong in the implementation, not in the public API comment.

# README and knowledge have different jobs

* **README** is the public-facing overview: value proposition, quick examples,
  installation, scope, license.
* **This knowledge bundle** is the source of truth for design decisions, rules, and
  contracts.

The README is never hidden inside the bundle, and design rationale is not duplicated at
length in the README — link to the relevant concept instead.

# Knowledge bundle conventions

This bundle targets **OKF v0.2**. `knowledge/index.md` is the bundle root and the only file
that declares `okf_version`.

* Reserved filenames are `index.md` (directory listing) and `log.md` (chronological
  history). Every other `.md` file is a concept.
* Every concept starts with a YAML frontmatter block containing a non-empty `type`.
* `type` values are human-readable phrases — for example `Architecture Decision`,
  `Design Principle`, `Project Rule`, `Requirement`, `API Contract`, `Test Strategy` —
  rather than machine-oriented abbreviations.
* Recommended metadata: `title`, `description`, `tags`.
* `status` is one of `draft`, `stable`, `deprecated`. An absent `status` means `stable`.
* `generated` and `verified` record provenance; see
  [AI-Assisted Development Rules](/rules/ai-assisted-development.md).
* `sources` records the material a concept was derived from, such as the originating issue.
* `stale_after` is used only for time-dependent knowledge. It is not attached mechanically
  to durable design decisions.
* Cross-links use bundle-relative absolute paths, for example
  `/decisions/adr-0001-cpp20.md`.
* Keep each concept small and focused on one main topic.

# Directories are created when they are needed

Directories are added when the first concept that belongs in them is written. Empty
taxonomy directories are not created in advance.

# ADR conventions

Decisions that are expensive or irreversible are recorded in `knowledge/decisions/`.

* File name: `adr-NNNN-<kebab-case-slug>.md`, with `NNNN` zero-padded to four digits.
* Numbers are assigned in order and never reused, including for superseded ADRs.
* A superseded ADR is kept and its frontmatter `status` becomes `deprecated`, with a link
  to the ADR that replaces it.
* Every ADR body contains at least: **Context**, **Decision**, **Alternatives considered**,
  **Consequences**, **Status**, **Related**.
* The body `Status` uses ADR vocabulary (for example `Accepted`, `Superseded`). The
  frontmatter `status` uses the OKF lifecycle vocabulary and describes the document, not
  the decision.

Not every change needs an ADR. Record one when a future contributor would otherwise have to
re-derive why an option was rejected.

# When documentation must be updated

Knowledge is updated when design knowledge actually changes. A pure internal refactoring
that changes no contract, rule, or decision does not require a knowledge edit. Pull requests
state whether a knowledge or ADR update was needed, and why.
