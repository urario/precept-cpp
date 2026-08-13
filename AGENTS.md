# Development instructions

These instructions apply to every contributor. Agents (Codex, Claude Code, or any other) work from
the same commands and the same rules as humans; there is no agent-only workflow, and no tool-specific
file is a source of truth.

[CONTRIBUTING.md](CONTRIBUTING.md) is the full guide — build, test, review criteria, pull requests.
Four points matter most when starting a task here.

## 1. Start from the knowledge bundle

Read [`knowledge/index.md`](knowledge/index.md) first, then the ADRs, rules, and contracts relevant
to the task. It is the source of truth for design decisions, and reviews cite it.

For a change to public API, the gate is
[API Admission Rules](knowledge/architecture/api-admission-rules.md); the shape is
[Design Principles](knowledge/architecture/design-principles.md); the v0.1 boundary is
[ADR-0005](knowledge/decisions/adr-0005-v0-1-span-scope.md).

## 2. Build, test, and check with the standard commands

`cmake` → `ctest`, exactly as in [CONTRIBUTING.md](CONTRIBUTING.md#start-here). Do not add a
parallel agent-only script.

The plain quickstart does not guarantee the knowledge check ran: without Python and PyYAML, CMake
silently skips registering it and `ctest` still passes. The
[agent task workflow](knowledge/rules/ai-assisted-development.md) expects the knowledge check to
run together with the rest of the test suite, so configure with
`-DPRECEPT_REQUIRE_KNOWLEDGE_CHECK=ON` after installing PyYAML — see
[Knowledge check](CONTRIBUTING.md#knowledge-check) — instead of relying on the plain quickstart.

## 3. Do not decide open design questions

If the task needs a design decision that is not already recorded in an issue or a concept, raise it
in an issue and leave it open. Knowledge concepts summarize decisions that were already made — they
are not where a decision gets made, and `status: draft` does not make a placeholder acceptable.

## 4. Record provenance honestly

Follow [AI-Assisted Development Rules](knowledge/rules/ai-assisted-development.md): record
`generated` with the version the tool actually reports and the real time of the change, or omit it;
never add a `human:` verification on someone's behalf; leave AI-drafted knowledge at
`status: draft`. Unknown is better than invented.

## Language

Issues, pull requests, and reviews: Japanese by default, English very welcome. Repository artifacts,
including the knowledge bundle: English. Translations may sit beside an English original, which
stays normative.
