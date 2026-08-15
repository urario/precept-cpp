---
type: Architecture Decision
title: Define versioning, compatibility, deprecation, and platform support for v0.1.0
description: Adopt SemVer with a MINOR-boundary compatibility promise, a one-MINOR deprecation notice, and CI-verified platform support ahead of v0.1.0.
status: draft
sources:
  - id: issue-26
    resource: https://github.com/urario/precept-cpp/issues/26
    title: Foundation issue for versioning policy, API stability, and supported platform scope
    author: claude-code/2.1.231
  - id: issue-26-decision-comment
    resource: https://github.com/urario/precept-cpp/issues/26#issuecomment-5290846380
    title: Owner decision on versioning, compatibility, deprecation, and support, recorded from a chat conversation
    author: claude-code/2.1.231
  - id: pr-71-review
    resource: https://github.com/urario/precept-cpp/pull/71#pullrequestreview-4943817959
    title: Owner review requiring header-path changes to be covered by the compatibility policy
    author: human:urario
tags: [architecture, versioning, compatibility, deprecation, support, release]
---

# Context

Precept is approaching its first tagged release. README currently says only that "the public API
is not frozen yet," which does not tell a consumer what a tag promises, what counts as a breaking
change, how a name is retired, or what it means for a platform to be "supported" when the
maintainer does not own every machine in the CI matrix (there is no macOS hardware). Issue #26
raised these as open questions before v0.1.0 ships.

The installed CMake package already implements

```cmake
write_basic_package_version_file(
  ...
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMinorVersion
  ARCH_INDEPENDENT)
```

(#9), which presumes same-MINOR compatibility at the packaging level without a project-wide policy
decision behind it. CONTRIBUTING.md also already states that clang-tidy "is not a required gate.
Introducing one requires a separate project decision," leaving that question open as well.

# Decision

Precept adopts Semantic Versioning (SemVer) for tagged releases, starting at 0.1.0.

Within the 0.x series, a MINOR version bump (0.1 → 0.2) is the compatibility boundary: a PATCH
release (0.1.0 → 0.1.1) must not break existing callers, while a MINOR release may. This matches
the compatibility already assumed by the installed CMake package's `SameMinorVersion` policy.

A change counts as breaking when it does any of the following:

* removes or renames a public type or function
* removes or renames a shipped public header path such that an existing `#include` no longer works
* narrows the set of inputs `try_from` (and equivalent validating factories) accept
* removes an existing conversion between two public types
* changes `std::ranges` concept modeling that a public type provides (for example, revoking
  `enable_borrowed_range`)
* weakens an existing `noexcept` or `constexpr` guarantee

Removing a public name is deprecated for at least one MINOR release before removal, using
`[[deprecated("...")]]` to carry a machine-visible notice.

Relocating a shipped public header follows the same one-MINOR notice principle, but an include path
cannot carry `[[deprecated]]`. Introduce the new canonical path while retaining the old path as a
forwarding header for at least one MINOR release. The forwarding header contains no duplicate public
declarations; it includes the canonical header. Release or migration documentation identifies the
preferred path and planned removal. The old path is removed only at a compatibility boundary where
a breaking change is permitted.

A platform or compiler is "supported" when it is verified by a required CI job, not when the
maintainer owns the hardware. README does not distinguish CI-only-verified platforms from others;
once #28 adds the remaining CI jobs, every platform in the required matrix is listed the same way.

Minimum supported compiler versions are not fixed by this ADR. They are established empirically in
#28 by adding CI jobs for older toolchains, and are written into README only once a job has
actually passed against them.

clang-tidy becomes a required CI gate. Its introduction is scoped to #27, separate from this
policy decision.

# Alternatives considered

Issue #26 presented these alternatives, and the owner chose among them in the linked decision
comment:

* For versioning: a project-specific scheme without PATCH, or no documented scheme at all.
  Rejected in favor of SemVer for its familiarity and its alignment with the CMake package version
  compatibility already implemented.
* For the 0.x compatibility boundary: treating every 0.x release as unconditionally breaking, or
  deciding compatibility case by case per issue. Rejected because both would leave the
  already-implemented `SameMinorVersion` CMake policy without a matching project-level rule.
* For deprecation: no notice period, or withholding any deprecation mechanism until 1.0. Rejected
  in favor of a one-MINOR `[[deprecated]]` notice; the cost is judged near zero for a header-only
  library of borrowed-view types.
* For the supported-platform basis: requiring maintainer-owned hardware for every listed platform.
  Rejected as impractical — no macOS hardware is available — in favor of CI verification as the
  basis.
* For minimum compiler versions: fixing candidate version numbers in this ADR before they are
  verified in CI. Rejected in favor of leaving them open until #28 establishes them empirically.
* For clang-tidy: leaving the decision open with a re-evaluation trigger, or rejecting it outright.
  The owner chose to require it now, overriding the AI-drafted recommendation to leave the decision
  open.

The later public-header-layout review added a compatibility clarification rather than replacing the
original versioning decision: a shipped header path is part of the public API surface, and relocation
uses a forwarding-header transition before removal.

# Consequences

* README's "the public API is not frozen yet" language is replaced with the compatibility promise
  above before v0.1.0 ships (#30).
* #27's scope, previously excluding clang-tidy pending this decision, now includes introducing it
  as a required CI gate.
* #28 proceeds with its own scope (libc++/macOS CI jobs, minimum compiler verification, README
  wording) without further dependency on this ADR beyond what is decided here.
* Deprecating a public name going forward requires an accompanying `[[deprecated("...")]]`
  attribute and at least one MINOR release before removal.
* Relocating a shipped public header requires a forwarding header at the old path for at least one
  MINOR release before that path can be removed.
* README's platform-support section, once #28 lands, lists every CI-verified platform uniformly,
  without singling out platforms the maintainer does not personally run.

# Status

Accepted.

# Related

* Issue [#26](https://github.com/urario/precept-cpp/issues/26) — versioning, API stability, and
  support-scope decision issue
* Issue [#26 decision comment](https://github.com/urario/precept-cpp/issues/26#issuecomment-5290846380)
  — recorded owner decision
* Issue [#27](https://github.com/urario/precept-cpp/issues/27) — CI quality gates, including
  clang-tidy adoption
* Issue [#28](https://github.com/urario/precept-cpp/issues/28) — standard library coverage,
  minimum compiler verification, README support section
* Issue [#30](https://github.com/urario/precept-cpp/issues/30) — v0.1.0 release
* Issue [#70](https://github.com/urario/precept-cpp/issues/70) — public header layout and include-tree
  architecture review
* [Public Header Layout Rule](../architecture/public-header-layout.md)
* [Project Charter](../vision/project-charter.md)
