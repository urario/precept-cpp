---
type: Project Rule
title: AI-Assisted Development Rules
description: How AI agents participate in Precept development and how knowledge provenance and trust are recorded.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-14T07:45:00Z
sources:
  - id: issue-3
    resource: https://github.com/urario/precept-cpp/issues/3
    title: Foundation issue for contribution, documentation, and AI-assisted development rules
    author: human:urario
  - id: issue-10
    resource: https://github.com/urario/precept-cpp/issues/10
    title: Foundation issue for managing project knowledge, rules, and ADRs with OKF v0.2
    author: human:urario
  - id: issue-31
    resource: https://github.com/urario/precept-cpp/issues/31
    title: Design issue for the agent's issue-update responsibilities in the task workflow
    author: claude-code/2.1.231
  - id: issue-31-decision-comment
    resource: https://github.com/urario/precept-cpp/issues/31#issuecomment-5290847240
    title: Owner decision on agent issue-update autonomy, body-edit scope, and update triggers, recorded from a chat conversation
    author: claude-code/2.1.231
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
5. Runs the knowledge check together with the rest of the test suite.
6. States in the pull request whether a knowledge or ADR update was needed, and why.
7. Updates the issues the task touches, following the boundaries in
   "Issue update responsibilities" below.

# Issue update responsibilities

An issue is the current-state record of a plan, not just a prompt for a pull request — a Done-when
checklist that stops being updated once implementation starts stops being useful. Agents keep
issues current within the same autonomy boundaries as any other action.

Always, without asking first:

* Add a fact-based status comment to any issue — progress, CI state, dependency state. This covers
  a comment made when starting a task as much as one made along the way.
* Update the body of an issue the agent itself authored — reflect a decision, check off a
  completed task or Done-when item.
* File a new issue for a follow-up or open question discovered mid-task.

Confirm first — propose in a comment, apply only after the change is approved:

* Changing the state (for example, closing) of an issue the agent did not author.
* Changing dependencies or scope, such as splitting or merging issues.

Never: recording an invented design decision as settled. See "Agents do not invent design
decisions" below — that boundary applies to issue updates exactly as it applies to knowledge.

Body edits are limited to issues the agent itself authored. An issue authored by someone else is
updated through a comment, never a body rewrite: fetching a body through the GitHub API and
writing it back can silently corrupt code blocks — angle brackets round-trip through HTML-entity
escaping — and an agent holding the exact text it originally wrote is the one case where that risk
does not apply.

The roadmap issue ([#1](https://github.com/urario/precept-cpp/issues/1)) is updated at meaningful
checkpoints — a Phase completing, a new epic being filed — not on every commit or every session.

# Agents do not invent design decisions

An agent must not silently resolve an open design question. If a task requires a decision that is
not already recorded in an issue or a concept, the agent raises it in an issue and leaves it open
until it is settled.

Open design questions live in issues and discussions; concepts hold knowledge that is settled. A
concept is not created to store an undecided question, and `status: draft` does not make one
acceptable — `draft` means knowledge that has not been reviewed yet, not a placeholder for a
decision nobody has made. Linking from a concept to the open issue that owns a question is
correct, and preferable to restating the question as knowledge.

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

Trust signals follow OKF v0.2 and its actor convention:

| Actor | Form | Example |
|-------|------|---------|
| Agent or tool | `<producer>/<version>` | `claude-code/2.1.231` |
| Person | `human:<id>` | `human:urario` |
| Automated process | `process:<id>` | `process:nightly-link-check` |

`process:` identifies an automated process such as a scheduled job or a pipeline step. It is not a
fallback for an interactive coding agent: an agent that cannot determine its own version does not
become a process by declaring itself one.

* `generated.by` identifies who or what produced the current content. An agent records itself in
  `<producer>/<version>` form, using the version the tool itself reports — never a guessed,
  rounded, or placeholder version.
* `generated.at` is the timestamp of the current content's last meaningful change. It is not the
  pull request date, not a time rounded to `00:00:00`, not a time an agent recalls, and not a
  timestamp left over from an earlier version whose body has since been rewritten.
* The whole `generated` family is optional. When either the exact producer or the exact time of
  the last meaningful change cannot be established, `generated` is omitted rather than
  approximated. An omitted field is accurate; an invented one is a false record that a later
  reader cannot tell apart from a true one. Git history remains the full account of who changed
  what and when.
* `verified` records confirmation, and a `human:` entry is added **only after a human has
  actually reviewed the content**. An agent never adds a `human:` verification on someone's
  behalf.
* A concept with no `verified` entry is unverified. That is an accurate state, not a defect.
* AI-drafted knowledge is not treated as `stable` or human-reviewed by default. It stays
  `status: draft` until a human reviews it.
* A concept whose body is materially rewritten goes back to `status: draft` with no human
  `verified` entry, even when the previous version had one. A review confirmed the text that was
  reviewed, not the text that replaced it. Corrections that touch only metadata do not discard an
  existing human verification.
* `sources` points at the issues, pull requests, or external material the content came from,
  so a reviewer can check the summary against its origin.

The rule behind all of this: **AI may summarize evidence; AI must not manufacture provenance.**
Unknown is better than invented.

# Reviewing agent-authored knowledge

When a human reviews an AI-drafted concept and accepts it, the reviewer — not the agent —
updates the frontmatter: `status` moves to `stable`, and a `verified` entry with a `human:`
actor and timestamp is added.
