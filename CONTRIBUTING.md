# Contributing to Precept

日本語版は [CONTRIBUTING.ja.md](CONTRIBUTING.ja.md) にあります。

This English version is normative. Where the translation disagrees with it, this file governs.

## Why contribute

Precept turns semantic preconditions — the `size() >= 16` buried in a function body — into small
types that state the condition in the signature and carry it across API boundaries. The
[Project Charter](knowledge/vision/project-charter.md) explains what that buys and what it
deliberately refuses to become.

Two things make it a comfortable project to contribute to:

* **The scope is small.** v0.1 is four span APIs, not a framework.
* **Nothing is decided in someone's head.** Every rule your change will be reviewed against is
  written down in the [knowledge bundle](knowledge/index.md) and linked from this page.

## Language

Issues, pull requests, reviews, and discussions are written primarily in **Japanese**. English is
very welcome; nobody is asked to switch.

Repository artifacts — README, code comments, public API documentation, identifiers, examples,
user-facing text, and the knowledge bundle — are written in **English**. The full policy is in the
[Documentation Rules](knowledge/rules/documentation.md).

## Start here

```sh
git clone https://github.com/urario/precept-cpp.git
cd precept-cpp
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The first configure needs network access so CMake can fetch the pinned GoogleTest release.

For multi-configuration generators such as Visual Studio, select the same configuration when
building and testing:

```sh
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

A library-only configuration neither fetches nor builds GoogleTest:

```sh
cmake -S . -B build-no-tests -DBUILD_TESTING=OFF
cmake --build build-no-tests
```

### Knowledge check

`tools/check_knowledge.py` validates the knowledge bundle and runs under CTest alongside
everything else — but only when Python 3 and PyYAML are available. Without them, CMake reports
that at configure time and simply does not register the knowledge tests, so a plain `ctest` run
from [Start here](#start-here) can pass without ever checking the bundle. To run it standalone
while editing knowledge:

```sh
python -m pip install pyyaml==6.0.3
python tools/check_knowledge.py knowledge
```

To get the same guarantee CI has — the knowledge tests actually running, not silently skipped —
configure with `-DPRECEPT_REQUIRE_KNOWLEDGE_CHECK=ON` after installing PyYAML:

```sh
python -m pip install pyyaml==6.0.3
cmake -S . -B build -DBUILD_TESTING=ON -DPRECEPT_REQUIRE_KNOWLEDGE_CHECK=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

This turns a missing interpreter or missing PyYAML into a configuration error instead of a silent
skip. Both remain development-only and never become requirements of the library.

### Formatting

`.clang-format` defines the source style. Today it covers only `tests/*.cpp` — there are no
production headers yet. Check or apply it with:

```sh
clang-format --dry-run --Werror tests/*.cpp
clang-format -i tests/*.cpp
```

clang-tidy is not a required gate yet; it is reconsidered when the public API is introduced.

## Before you write code

1. Read the [knowledge index](knowledge/index.md).
2. Read the ADRs, rules, and contracts relevant to your change.

This is the step that saves a rewrite. The knowledge bundle is the source of truth for design
decisions, and it is what reviewers cite.

## What gets merged

* **Public API** satisfies all eight [API Admission Rules](knowledge/architecture/api-admission-rules.md).
  Failing one is a reason to reshape the proposal, not a follow-up task — so propose new API in an
  issue before implementing it. Admitted API is then shaped by the
  [Design Principles](knowledge/architecture/design-principles.md).
* **Scope** stays inside v0.1: span size preconditions, per
  [ADR-0005](knowledge/decisions/adr-0005-v0-1-span-scope.md). Ideas beyond it are welcome as
  issues.
* **Base policy** holds: C++20, header-only, zero consumer dependencies
  ([ADR-0001](knowledge/decisions/adr-0001-cpp20.md),
  [ADR-0002](knowledge/decisions/adr-0002-header-only.md)). Development-only tooling never becomes
  a consumer requirement.
* **Source files** start with the SPDX license header from the
  [Coding Rules](knowledge/rules/coding.md) and are `clang-format` clean.
* **CI is green**: Linux GCC, Linux Clang, and Windows MSVC, each running the knowledge check.

## Tests

CTest is the common execution entry point; each kind of property goes to the smallest mechanism
that can hold it:

* runtime behavior → GoogleTest
* compile-time properties → `static_assert`
* positive compile tests → ordinary CMake build targets
* negative compile tests → only when a public API constraint needs its rejection verified

The full split is in the [Test Strategy](knowledge/testing/test-strategy.md).

## Knowledge and ADRs

Update the knowledge bundle when design knowledge actually changed. A refactoring that changes no
contract, rule, or decision needs no knowledge edit — "not needed, because …" is a complete answer
in the pull request.

Write an ADR when a future contributor would otherwise have to re-derive why an option was
rejected. An ADR records a decision already reached in an issue, pull request, or review; it is not
where the decision gets made. Conventions are in the
[Documentation Rules](knowledge/rules/documentation.md).

## Open questions go to issues

If your change needs a design decision that is not recorded anywhere, raise it in an issue rather
than settling it inside the pull request. Open design questions live in issues and pull request
discussion until they are settled; the knowledge bundle holds what is already decided.

## AI-assisted contributions

AI assistance is a normal part of development here — Codex locally, Claude Code in the cloud.
Neither is required, and there is no agent-only workflow: the commands above are the source of
truth for humans and agents alike.

Two requirements are specific to agents: an agent does not invent design decisions, and provenance
metadata is recorded honestly, never guessed. Both are in the
[AI-Assisted Development Rules](knowledge/rules/ai-assisted-development.md). Agents start from
[AGENTS.md](AGENTS.md).

## Pull requests

Keep them small, and open them as drafts early if you want feedback on direction.

The pull request template asks for: related issue, what and why, public API change, tests,
documentation, **knowledge / ADR update and the reason**, and breaking change. Write the body in
Japanese or English.

If you changed `CONTRIBUTING.md`, update [CONTRIBUTING.ja.md](CONTRIBUTING.ja.md) in the same pull
request.
