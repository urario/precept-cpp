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

The source issue records the choice of Apache-2.0 but not the comparison behind it. The
alternatives below were reconstructed while drafting this ADR; they are the drafter's
reasoning, not a decision already recorded elsewhere, and they are open to correction or
removal in review.

* **MIT** — shorter and very widely understood, but contains no express patent grant.
  For a library about semantic API contracts that organizations embed directly, the
  explicit grant and its termination clause are worth the extra length. Rejected.
* **BSL-1.0** — common in the C++ ecosystem and does not require attribution in binaries,
  which is convenient for header-only libraries. Still lacks an explicit patent grant.
  Rejected for the same reason.
* **LGPL or MPL** — file-level or library-level copyleft interacts poorly with header-only
  distribution, where consumer binaries inevitably contain Precept's code. This would
  discourage exactly the adoption Precept needs. Rejected.

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
