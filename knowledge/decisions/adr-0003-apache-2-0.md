---
type: Architecture Decision
title: License Precept under Apache-2.0
description: Use Apache-2.0 so adoption is permissive and the patent grant is explicit.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-13T00:00:00+09:00
sources:
  - id: issue-1
    resource: https://github.com/urario/precept-cpp/issues/1
    title: Roadmap issue defining Precept's background, goals, principles, and roadmap
    author: human:urario
tags: [licensing, apache-2.0, governance]
---

# Context

Precept is intended to be embedded in other people's C++ code, including proprietary and
commercial code. The license is effectively irreversible for code already released, so it
belongs in the decision record.

The library is header-only, which means consumers compile Precept's source into their own
binaries. Any license term that is triggered by linking or by distributing derived object
code therefore applies broadly and must be considered carefully.

# Decision

Precept is licensed under Apache-2.0.

# Alternatives considered

The source material does not record alternatives considered for this decision.
No alternatives are reconstructed here.

# Consequences

* Commercial and proprietary use is permitted without a reciprocal source obligation.
* Contributors grant patent rights covering their contributions, and that grant terminates
  for a party that initiates patent litigation over the work.
* Consumers must preserve the license and notice files; this is a normal obligation but is
  worth stating in the README's license section.
* Relicensing later would require agreement from all contributors, so this decision is
  treated as effectively permanent.

## Required follow-up

The repository does **not** yet contain a `LICENSE` file. Until it does, this ADR records an
intent, not the repository's licensing state:

* Add `LICENSE` containing the Apache-2.0 text.
* State the license in the README.
* Keep any license notices in source files consistent with that file.

# Status

Accepted.

# Related

* Issue [#1](https://github.com/urario/precept-cpp/issues/1) — roadmap and base policy
* Issue [#8](https://github.com/urario/precept-cpp/issues/8) — README, examples, and docs
* [Project Charter](../vision/project-charter.md)
