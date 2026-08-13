---
type: Project Rule
title: AI-Assisted Development Rules
description: How AI agents participate in Precept development and how knowledge provenance and trust are recorded.
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
tags: [rules, ai-assisted-development, provenance]
---

# Assumed environments

Precept is developed with AI assistance as a normal part of the workflow — Codex locally
and Claude Code in the cloud. Neither is required to contribute.

There is no hidden agent-only workflow. Humans and agents use the same build, test, and
check commands, and those commands remain the source of truth.

# Agent task workflow

A contributor — human or agent — starting a task normally:

1. Reads [the knowledge index](../index.md).
2. Reads the Requirements, ADRs, Rules, and API Contracts relevant to the issue.
3. Implements and tests the change.
4. Updates the affected concepts **only if design knowledge changed**.
5. States in the pull request whether a knowledge or ADR update was needed, and why.

A knowledge check step will be added to this workflow once a checker exists; no checker is
part of the repository yet.

# Agents do not invent design decisions

An agent must not silently resolve an open design question. If a task requires a decision
that is not already recorded in an issue or a concept, the agent either raises it in an
issue or records it as a `status: draft` concept that states the open question — never as
settled knowledge.

Knowledge concepts summarize decisions that were already made in issues, pull requests, or
reviews. They are not a place to introduce new ones.

# AI may summarize evidence; AI must not manufacture project history

Rationale, alternatives, and consequences that the source material does not record are left
out, not reconstructed. Labeling reconstructed reasoning is not sufficient: knowledge is
retrieved in fragments, and a fragment separated from its label reads as history the project
actually has — an option someone weighed and rejected, when nobody did.

Where a section is required by convention and the sources record nothing behind it, say so
explicitly rather than filling it in.

# Provenance and trust

Trust signals follow OKF v0.2 and the actor convention `human:<id>`, `process:<id>`, and
`<producer>/<version>` for agents.

* `generated.by` identifies who or what produced the current content. Concepts drafted by
  an agent record the agent in `<producer>/<version>` form, for example
  `claude-code/2.1.231`. The version is the tool version actually used, reported by the
  tool itself — it is never guessed, rounded, or left as a placeholder. An agent that
  cannot determine its own version records `process:<id>` instead and says so in the pull
  request, rather than inventing a version.
* `verified` records confirmation, and a `human:` entry is added **only after a human has
  actually reviewed the content**. An agent never adds a `human:` verification on someone's
  behalf.
* A concept with no `verified` entry is unverified. That is an accurate state, not a defect.
* AI-drafted knowledge is not treated as `stable` or human-reviewed by default. It stays
  `status: draft` until a human reviews it.
* `sources` points at the issues, pull requests, or external material the content came from,
  so a reviewer can check the summary against its origin.

# Reviewing agent-authored knowledge

When a human reviews an AI-drafted concept and accepts it, the reviewer — not the agent —
updates the frontmatter: `status` moves to `stable`, and a `verified` entry with a `human:`
actor and timestamp is added.
