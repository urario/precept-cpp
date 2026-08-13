---
type: Project Rule
title: Documentation Rules
description: Language policy, the split between README and this knowledge bundle, and how knowledge concepts are authored.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-13T22:41:00Z
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

A translation may sit beside an English artifact — `CONTRIBUTING.ja.md` next to
`CONTRIBUTING.md` — so that a contributor reads the guide in the language they think in. The
English version stays **normative**: where the two disagree, the English text governs, and a
translation is updated in the same pull request that changes its original. A translation never
carries a rule that its original does not.

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

## OKF conformance is not the same thing as Precept repository validity

A bundle can conform to OKF v0.2 and still be unacceptable in this repository, and a Precept rule
can be broken without any part of the specification being violated. Findings are therefore
reported at three severities that are never merged, and a Precept requirement is never described
as an OKF specification violation.

### OKF conformance error

A violation of the
[OKF v0.2 specification](https://github.com/GoogleCloudPlatform/knowledge-catalog/blob/main/okf/SPEC.md)
itself. Its conformance section defines exactly three requirements, and nothing else belongs at
this severity:

* Every non-reserved `.md` file contains a parseable YAML frontmatter block.
* Every frontmatter block contains a non-empty `type`.
* The reserved filenames follow their specified structure **when present**: `index.md` is section
  headings with link bullets and carries no frontmatter, except that the bundle root may declare
  `okf_version`; `log.md` groups entries under ISO `YYYY-MM-DD` date headings.

The specification is permissive by design, and Precept tooling is permissive in exactly the same
places. A bundle is never rejected for an unknown `type`, unrecognized extra frontmatter keys,
missing optional or recommended metadata, broken cross-links, or a missing `index.md`.

### Precept policy error

A requirement that OKF leaves optional or producer-defined and that this repository has decided
to enforce. Breaking one fails the check — as a Precept failure, not as a conformance failure:

* The bundle root `knowledge/index.md` exists. OKF makes every `index.md` optional; this project
  does not, because it is the entry point humans and agents are told to start from.
* The bundle root declares `okf_version`, as exactly the string `"0.2"`.
* ADR files follow the file name convention below.
* Every ADR body contains the mandatory sections below.

`log.md` is deliberately absent from that list. Both reserved filenames are optional in OKF, and
this bundle has none; its structure is checked only when the file exists. One is introduced once
the bundle needs a curated semantic history — important decisions, deprecations, releases,
significant knowledge updates — rather than a second rendering of the git log.

### Advisory warning

A quality problem worth a reviewer's attention that must not fail a build: a broken
repository-relative link, missing recommended metadata, an expired `stale_after`,
machine-generated content presented as `stable` with nothing verifying it, an actor or lifecycle
value outside the vocabularies below, or another policy deviation better discussed than enforced.

Warnings stay few on purpose. A check that reports something on every run stops being read, and
the errors mixed in with it are what get lost first.

## Authoring conventions

* `type` values are human-readable phrases — for example `Architecture Decision`,
  `Design Principle`, `Project Rule`, `Requirement`, `API Contract`, `Test Strategy` —
  rather than machine-oriented abbreviations. OKF registers no types, so an unfamiliar one is
  never an error; it is a naming question for review.
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

## The knowledge checker

`tools/check_knowledge.py` checks this bundle at the three severities above. It runs under CTest
alongside the C++ tests, so a normal test run covers the knowledge bundle too, and it can be run
on its own during editing.

It is development-only tooling. The interpreter and YAML parser it needs are never requirements of
the library, never propagate to consumers, and are absent from a library-only configuration.

It is deliberately not a general OKF validator: it checks what this repository needs and tolerates
everything the specification tolerates. The judgements that need a reader — whether a concept
records a decision that was actually made, whether an ADR reconstructs reasoning it does not have,
whether provenance is honest — belong to review and are not automated. A clean run means no rule
was broken, not that the knowledge is correct.

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

External references use full URLs. The knowledge checker resolves links by this same rule.

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
