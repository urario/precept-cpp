---
type: Architecture Decision
title: License Precept under Apache-2.0
description: Use Apache-2.0 so adoption is permissive and the patent grant is explicit.
status: draft
generated:
  by: claude-code/2.1.231
  at: 2026-08-13T19:24:27Z
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
* Each contributor grants a patent license under Section 3, limited to the patent claims that
  contributor is able to license and that are necessarily infringed by that contributor's own
  contribution alone, or by the combination of that contribution with the work it was submitted
  to. It reaches no further: it says nothing about patents held by anyone who is not a
  contributor, and it is neither third-party patent clearance nor a freedom-to-operate analysis
  for users of Precept. The grant terminates for a party that institutes patent litigation
  alleging that the work, or a contribution incorporated within it, infringes a patent.
* Redistributors carry the Section 4 obligations: give recipients a copy of the license, mark
  modified files as changed, and keep the copyright, patent, trademark, and attribution notices
  present in the source form. The further NOTICE obligation in Section 4(d) applies only when the
  work is distributed with a `NOTICE` text file. This ADR does not decide to add one; adding one
  later would extend that obligation to downstream derivative works.
* Relicensing the work under incompatible terms later would require permission from the relevant
  copyright holders, which becomes progressively harder as contributors accumulate. The choice is
  therefore treated as effectively permanent.

This summary is written for readers of this repository. The license text itself is what governs,
and none of it is legal advice.

## Follow-up

This ADR records a license choice; it does not by itself license the repository. Adoption is
complete when the Apache-2.0 text is present as `LICENSE` at the repository root, the README
states the license — owned by issue [#8](https://github.com/urario/precept-cpp/issues/8) — and any
per-file notices are consistent with that file. The decision recorded here and the licensing state
of the repository are separate things, and neither should be inferred from the other.

# Status

Accepted.

# Related

* Issue [#1](https://github.com/urario/precept-cpp/issues/1) — roadmap and base policy
* Issue [#8](https://github.com/urario/precept-cpp/issues/8) — README, examples, and docs
* [Project Charter](../vision/project-charter.md)
