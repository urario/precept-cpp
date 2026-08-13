---
type: Project Rule
title: Documentation Rules
description: Language policy, the split between README and this knowledge bundle, and how knowledge concepts are authored.
status: draft
generated:
  by: claude-code/2.1.231
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

Two separate layers of rules apply, and they are deliberately never merged: OKF v0.2
conformance is defined by the specification, while everything else is Precept's own policy.
Any future knowledge checker must report the first layer as errors and the second as
warnings.

## Layer 1 — OKF v0.2 conformance

These requirements come from the
[OKF v0.2 specification](https://github.com/GoogleCloudPlatform/knowledge-catalog/blob/main/okf/SPEC.md).
A bundle conforms when:

* Every non-reserved `.md` file contains parseable YAML frontmatter.
* Every frontmatter block contains a non-empty `type`.
* Reserved files follow the structure the specification defines for them: `index.md` is
  section headings plus link bullets, and `log.md` is date-grouped entries, newest first.
* `okf_version` is declared only in the bundle-root `index.md`.

The specification is permissive by design. It does **not** reject a bundle for unknown
`type` values, unrecognized extra frontmatter keys, missing optional or recommended
metadata, broken cross-links, or missing `index.md` files. Precept tooling must not reject
a bundle for those reasons either.

Both reserved filenames are optional in OKF: `index.md` is a directory listing, `log.md` is
an update history. Precept requires the bundle-root `index.md` as project policy — that
requirement does not come from OKF.

This bundle has no `log.md`. A checker must not require one; it validates the structure only
when the file is present. A `log.md` is introduced once the bundle needs a curated semantic
history — important decisions, deprecations, releases, significant knowledge updates —
rather than a second rendering of the git log.

## Layer 2 — Precept project policy

These are this project's conventions. Breaking one is a warning to be discussed in review,
not an OKF conformance failure:

* `type` values are human-readable phrases — for example `Architecture Decision`,
  `Design Principle`, `Project Rule`, `Requirement`, `API Contract`, `Test Strategy` —
  rather than machine-oriented abbreviations.
* Recommended metadata on every concept: `title`, `description`, `tags`.
* `status` is one of `draft`, `stable`, `deprecated`. An absent `status` means `stable`.
* `generated` and `verified` record provenance; see
  [AI-Assisted Development Rules](ai-assisted-development.md).
* `sources` records the material a concept was derived from, such as the originating issue.
* `stale_after` is used only for time-dependent knowledge. It is not attached mechanically
  to durable design decisions.
* Cross-links follow the link style below.
* ADR files follow the ADR conventions below.
* Keep each concept small and focused on one main topic.

## Cross-link style

OKF recommends bundle-relative absolute paths such as `/decisions/adr-0001-cpp20.md`. This
bundle lives inside a git repository and is read most often through the repository's own
file view, where a leading `/` resolves against the host root rather than the bundle root,
which breaks the link for both humans and agents.

Precept therefore uses **repository-relative links** that resolve when the file is viewed
in place:

* from `knowledge/index.md`: `vision/project-charter.md`
* from a concept in a subdirectory: `../decisions/adr-0001-cpp20.md`
* within the same directory: `design-principles.md`

External references use full URLs. A future checker resolves links by this same rule.

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

## An ADR records decisions that were already made

An ADR writes down a decision reached in an issue, pull request, or review. It is not the
place to make one.

* Rationale, alternatives, and consequences that the ADR's `sources` do not record are
  omitted. They are not reconstructed, and a label such as "reconstructed while drafting"
  does not make reconstruction acceptable — knowledge is retrieved in fragments, and a
  fragment separated from its label reads as a decision the project actually made.
* Where the sources record that an option was rejected but not the comparison behind it,
  state only what the sources support.
* Reasoning that would settle a genuinely new question — release strategy, scope, product
  direction — is not added under a heading like "Alternatives considered". It goes to an
  issue and comes back once decided.

`Alternatives considered` stays in the ADR even when there is nothing to put in it. Say so
plainly instead of filling it in:

```markdown
# Alternatives considered

The source material does not record alternatives considered for this decision.
No alternatives are reconstructed here.
```

## An ADR describes the decision, not a repository state it does not have

Consequences that depend on files, targets, or workflows the repository does not contain
yet are written as requirements on the future work, with the owning issue named, in a
clearly separated section. They are never phrased as things that already hold — an agent
reading the bundle would otherwise treat unimplemented work as existing.

# When documentation must be updated

Knowledge is updated when design knowledge actually changes. A pure internal refactoring
that changes no contract, rule, or decision does not require a knowledge edit. Pull requests
state whether a knowledge or ADR update was needed, and why.
