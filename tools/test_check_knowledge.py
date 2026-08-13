"""Tests for the knowledge checker.

Run from the repository root:

    python -m unittest -q tools.test_check_knowledge
"""

from __future__ import annotations

import contextlib
import io
import tempfile
import unittest
from pathlib import Path

from tools import check_knowledge
from tools.check_knowledge import OKF_ERROR, POLICY_ERROR, WARNING

ROOT_INDEX = """---
okf_version: "0.2"
---

# Rules

* [Example](rules/example.md) - An example concept.
"""

CONCEPT = """---
type: Project Rule
title: Example Rule
description: An example concept used by the checker tests.
tags: [example]
---

# Body

Nothing to see here.
"""

ADR = """---
type: Architecture Decision
title: Example decision
description: An example ADR used by the checker tests.
tags: [example]
---

# Context

Something had to be decided.

# Decision

It was decided.

# Alternatives considered

The source material does not record alternatives considered for this decision.

# Consequences

Life goes on.

# Status

Accepted.

# Related

* Nothing.
"""

LOG = """# Update Log

## 2026-08-14
* **Update**: Something happened.

## 2026-08-13
* **Creation**: Something was created.
"""


def rules_at(report: check_knowledge.Report, severity: str) -> list[str]:
    return sorted(f.rule for f in report.findings if f.severity == severity)


class BundleTestCase(unittest.TestCase):
    """A minimal valid bundle that each test perturbs in exactly one way."""

    def setUp(self) -> None:
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.bundle = Path(temporary.name) / "knowledge"
        self.write("index.md", ROOT_INDEX)
        self.write("rules/example.md", CONCEPT)

    def write(self, relative: str, text: str) -> Path:
        path = self.bundle / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
        return path

    def check(self) -> check_knowledge.Report:
        report = check_knowledge.Report()
        check_knowledge.check_bundle(self.bundle, report)
        return report

    def assertClean(self, report: check_knowledge.Report) -> None:
        self.assertEqual([], [tuple(f) for f in report.findings])


class ValidBundleTests(BundleTestCase):
    def test_minimal_bundle_has_no_findings(self):
        self.assertClean(self.check())

    def test_missing_log_is_valid(self):
        self.assertFalse((self.bundle / "log.md").exists())
        self.assertClean(self.check())

    def test_unknown_type_is_valid(self):
        self.write("rules/example.md", CONCEPT.replace("Project Rule", "Some Unregistered Type"))
        self.assertClean(self.check())

    def test_extra_frontmatter_keys_are_valid(self):
        self.write("rules/example.md", CONCEPT.replace("tags: [example]", "tags: [example]\nowner: nobody"))
        self.assertClean(self.check())

    def test_valid_adr_and_log_have_no_findings(self):
        self.write("decisions/adr-0001-example.md", ADR)
        self.write("log.md", LOG)
        self.assertClean(self.check())


class OkfConformanceTests(BundleTestCase):
    def test_concept_without_type(self):
        self.write("rules/example.md", CONCEPT.replace("type: Project Rule\n", ""))
        self.assertEqual(["okf.type.missing"], rules_at(self.check(), OKF_ERROR))

    def test_concept_with_empty_type(self):
        self.write("rules/example.md", CONCEPT.replace("type: Project Rule", 'type: ""'))
        self.assertEqual(["okf.type.missing"], rules_at(self.check(), OKF_ERROR))

    def test_concept_without_frontmatter(self):
        self.write("rules/example.md", "# Body\n\nNo frontmatter at all.\n")
        self.assertEqual(["okf.frontmatter.missing"], rules_at(self.check(), OKF_ERROR))

    def test_unparseable_frontmatter(self):
        self.write("rules/example.md", "---\ntype: [unterminated\n---\n\n# Body\n")
        self.assertEqual(["okf.frontmatter.unparseable"], rules_at(self.check(), OKF_ERROR))

    def test_frontmatter_in_non_root_index(self):
        self.write("rules/index.md", ROOT_INDEX)
        self.assertEqual(["okf.index.frontmatter"], rules_at(self.check(), OKF_ERROR))

    def test_index_without_link_bullets(self):
        self.write("index.md", '---\nokf_version: "0.2"\n---\n\nJust a paragraph.\n')
        self.assertEqual(["okf.index.structure"], rules_at(self.check(), OKF_ERROR))

    def test_log_with_non_iso_date_heading(self):
        self.write("log.md", LOG.replace("## 2026-08-14", "## August 14, 2026"))
        self.assertEqual(["okf.log.date"], rules_at(self.check(), OKF_ERROR))

    def test_log_without_date_groups(self):
        self.write("log.md", "# Update Log\n\n* Something happened.\n")
        self.assertEqual(["okf.log.structure"], rules_at(self.check(), OKF_ERROR))

    def test_log_without_a_document_title(self):
        self.write("log.md", "# 2026-08-14\n* **Update**: Something happened.\n")
        self.assertClean(self.check())

    def test_log_out_of_order_is_a_warning_only(self):
        entries = LOG.replace("2026-08-14", "2026-08-01")
        self.write("log.md", entries)
        report = self.check()
        self.assertEqual([], rules_at(report, OKF_ERROR))
        self.assertEqual(["warn.log.order"], rules_at(report, WARNING))


class PolicyTests(BundleTestCase):
    def test_missing_root_index(self):
        (self.bundle / "index.md").unlink()
        report = self.check()
        self.assertEqual([], rules_at(report, OKF_ERROR))
        self.assertEqual(["policy.index.missing"], rules_at(report, POLICY_ERROR))

    def test_wrong_okf_version(self):
        self.write("index.md", ROOT_INDEX.replace('"0.2"', '"0.1"'))
        self.assertEqual(["policy.okf_version"], rules_at(self.check(), POLICY_ERROR))

    def test_unquoted_okf_version_is_not_the_required_string(self):
        self.write("index.md", ROOT_INDEX.replace('"0.2"', "0.2"))
        self.assertEqual(["policy.okf_version"], rules_at(self.check(), POLICY_ERROR))

    def test_index_without_okf_version(self):
        self.write("index.md", "# Rules\n\n* [Example](rules/example.md) - An example concept.\n")
        self.assertEqual(["policy.okf_version"], rules_at(self.check(), POLICY_ERROR))

    def test_bad_adr_filename(self):
        self.write("decisions/adr-1-example.md", ADR)
        self.assertEqual(["policy.adr.filename"], rules_at(self.check(), POLICY_ERROR))

    def test_adr_missing_mandatory_section(self):
        self.write("decisions/adr-0001-example.md", ADR.replace("# Alternatives considered", "# Notes"))
        report = self.check()
        self.assertEqual(["policy.adr.sections"], rules_at(report, POLICY_ERROR))
        self.assertEqual([], rules_at(report, OKF_ERROR))


class WarningTests(BundleTestCase):
    def test_broken_link_is_a_warning(self):
        self.write("rules/example.md", CONCEPT + "\nSee [nothing](../missing.md).\n")
        report = self.check()
        self.assertEqual([], report.errors)
        self.assertEqual(["warn.link.broken"], rules_at(report, WARNING))

    def test_link_inside_a_code_fence_is_ignored(self):
        self.write("rules/example.md", CONCEPT + "\n```markdown\n[nothing](../missing.md)\n```\n")
        self.assertClean(self.check())

    def test_missing_recommended_metadata(self):
        self.write("rules/example.md", "---\ntype: Project Rule\n---\n\n# Body\n")
        report = self.check()
        self.assertEqual([], report.errors)
        self.assertEqual(["warn.metadata.recommended"], rules_at(report, WARNING))

    def test_expired_stale_after(self):
        self.write("rules/example.md", CONCEPT.replace("tags: [example]", "tags: [example]\nstale_after: 2000-01-01"))
        self.assertEqual(["warn.stale"], rules_at(self.check(), WARNING))

    def test_unknown_status(self):
        self.write("rules/example.md", CONCEPT.replace("tags: [example]", "tags: [example]\nstatus: reviewed"))
        self.assertEqual(["warn.status.unknown"], rules_at(self.check(), WARNING))

    def test_actor_outside_the_okf_convention(self):
        self.write(
            "rules/example.md",
            CONCEPT.replace(
                "tags: [example]", "tags: [example]\nstatus: draft\ngenerated:\n  by: codex"
            ),
        )
        self.assertEqual(["warn.actor.convention"], rules_at(self.check(), WARNING))

    def test_machine_generated_stable_concept_without_verification(self):
        self.write(
            "rules/example.md",
            CONCEPT.replace(
                "tags: [example]",
                "tags: [example]\nstatus: stable\ngenerated:\n  by: some-agent/1.2.3\n  at: 2026-08-13T19:24:27Z",
            ),
        )
        self.assertEqual(["warn.unverified"], rules_at(self.check(), WARNING))

    def test_human_verified_concept_is_not_warned_about(self):
        self.write(
            "rules/example.md",
            CONCEPT.replace(
                "tags: [example]",
                "tags: [example]\nstatus: stable\nverified:\n  - by: human:urario\n    at: 2026-08-13T19:24:27Z",
            ),
        )
        self.assertClean(self.check())


class ExitStatusTests(BundleTestCase):
    def run_main(self) -> tuple[int, str]:
        stdout = io.StringIO()
        with contextlib.redirect_stdout(stdout):
            status = check_knowledge.main([str(self.bundle)])
        return status, stdout.getvalue()

    def test_valid_bundle_exits_zero(self):
        status, output = self.run_main()
        self.assertEqual(0, status)
        self.assertIn("0 OKF error(s), 0 policy error(s), 0 warning(s)", output)

    def test_warnings_alone_exit_zero(self):
        self.write("rules/example.md", CONCEPT + "\nSee [nothing](../missing.md).\n")
        status, output = self.run_main()
        self.assertEqual(0, status)
        self.assertIn("WARNING [warn.link.broken]", output)

    def test_okf_error_exits_non_zero(self):
        self.write("rules/example.md", CONCEPT.replace("type: Project Rule\n", ""))
        status, output = self.run_main()
        self.assertEqual(1, status)
        self.assertIn("OKF ERROR [okf.type.missing]", output)

    def test_policy_error_exits_non_zero(self):
        self.write("index.md", ROOT_INDEX.replace('"0.2"', '"0.1"'))
        status, output = self.run_main()
        self.assertEqual(1, status)
        self.assertIn("POLICY ERROR [policy.okf_version]", output)


if __name__ == "__main__":
    unittest.main()
