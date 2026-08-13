#!/usr/bin/env python3
"""Check the Precept knowledge bundle.

Findings are reported at three severities that are deliberately never merged:

  OKF ERROR     A violation of an Open Knowledge Format v0.2 conformance rule
                (specification section 11). Nothing else belongs here.
  POLICY ERROR  A requirement this repository imposes on itself, which OKF leaves
                optional or producer-defined.
  WARNING       Advisory. Reported for review, never fatal.

This is not a general OKF validator. It checks what the Precept repository needs and
tolerates everything OKF tolerates: unknown `type` values, extra frontmatter keys,
missing optional metadata, a missing `log.md`, and broken links never fail the check.

Usage:
    python tools/check_knowledge.py knowledge

Exit status: 1 if any error was reported, 2 on a usage or environment failure,
0 otherwise (warnings alone are not a failure).
"""

from __future__ import annotations

import argparse
import datetime
import re
import sys
from pathlib import Path
from typing import NamedTuple
from urllib.parse import unquote

try:
    import yaml
except ImportError:  # pragma: no cover - exercised only without the dev dependency
    yaml = None

OKF_ERROR = "OKF ERROR"
POLICY_ERROR = "POLICY ERROR"
WARNING = "WARNING"

SEVERITY_ORDER = {OKF_ERROR: 0, POLICY_ERROR: 1, WARNING: 2}

RESERVED_FILENAMES = ("index.md", "log.md")
RECOMMENDED_KEYS = ("title", "description", "tags")
STATUS_VALUES = ("draft", "stable", "deprecated")

ADR_DIRECTORY = "decisions"
ADR_FILENAME = re.compile(r"^adr-\d{4}-[a-z0-9]+(?:-[a-z0-9]+)*\.md$")
ADR_SECTIONS = (
    "Context",
    "Decision",
    "Alternatives considered",
    "Consequences",
    "Status",
    "Related",
)

ACTOR = re.compile(r"^(?:human:\S+|process:\S+|[^\s:/]+/\S+)$")
HEADING = re.compile(r"^(#{1,6})\s+(.+?)\s*#*$")
LIST_ITEM = re.compile(r"^\s*[*+-]\s+")
MARKDOWN_LINK = re.compile(r"\[[^\]]*\]\(\s*([^)\s]+)")
ISO_DATE = re.compile(r"^\d{4}-\d{2}-\d{2}$")


class Finding(NamedTuple):
    path: str
    severity: str
    rule: str
    message: str


class Report:
    """Collects findings and decides the exit status."""

    def __init__(self) -> None:
        self.findings: list[Finding] = []
        self.concepts = 0

    def add(self, path: Path, severity: str, rule: str, message: str) -> None:
        self.findings.append(Finding(path.as_posix(), severity, rule, message))

    @property
    def errors(self) -> list[Finding]:
        return [f for f in self.findings if f.severity != WARNING]

    @property
    def warnings(self) -> list[Finding]:
        return [f for f in self.findings if f.severity == WARNING]

    def write(self, stream) -> None:
        for finding in sorted(
            self.findings, key=lambda f: (f.path, SEVERITY_ORDER[f.severity], f.rule)
        ):
            print(
                f"{finding.path}: {finding.severity} [{finding.rule}] {finding.message}",
                file=stream,
            )
        okf = sum(1 for f in self.errors if f.severity == OKF_ERROR)
        policy = len(self.errors) - okf
        print(
            f"checked {self.concepts} concept document(s): "
            f"{okf} OKF error(s), {policy} policy error(s), {len(self.warnings)} warning(s)",
            file=stream,
        )


def split_frontmatter(text: str) -> tuple[str, str | None, str]:
    """Split a document into (kind, frontmatter, body).

    `kind` is "none" when the document has no frontmatter delimiter, "unterminated"
    when the opening delimiter is never closed, and "ok" otherwise.
    """
    text = text.removeprefix("\ufeff")
    lines = text.splitlines()
    if not lines or lines[0].strip() != "---":
        return "none", None, text
    for index in range(1, len(lines)):
        if lines[index].strip() in ("---", "..."):
            return "ok", "\n".join(lines[1:index]), "\n".join(lines[index + 1 :])
    return "unterminated", None, text


def without_code_fences(text: str) -> str:
    """Drop fenced code blocks so examples are not read as document structure."""
    kept: list[str] = []
    fence: str | None = None
    for line in text.splitlines():
        stripped = line.lstrip()
        if fence is None:
            if stripped.startswith("```") or stripped.startswith("~~~"):
                fence = stripped[:3]
                continue
            kept.append(line)
        elif stripped.startswith(fence):
            fence = None
    return "\n".join(kept)


def headings(body: str) -> list[tuple[int, str]]:
    found = []
    for line in without_code_fences(body).splitlines():
        match = HEADING.match(line)
        if match:
            found.append((len(match.group(1)), match.group(2).strip()))
    return found


def parse_frontmatter(report: Report, rel: Path, frontmatter: str) -> dict | None:
    """Parse a frontmatter block, reporting the OKF conformance failures it can have."""
    try:
        data = yaml.safe_load(frontmatter)
    except yaml.YAMLError as error:
        first_line = str(error).splitlines()[0]
        report.add(
            rel,
            OKF_ERROR,
            "okf.frontmatter.unparseable",
            f"frontmatter is not parseable YAML: {first_line}",
        )
        return None
    if data is None:
        return {}
    if not isinstance(data, dict):
        report.add(
            rel,
            OKF_ERROR,
            "okf.frontmatter.unparseable",
            "frontmatter is not a YAML mapping",
        )
        return None
    return data


def check_links(report: Report, bundle: Path, path: Path, rel: Path, text: str) -> None:
    for match in MARKDOWN_LINK.finditer(without_code_fences(text)):
        target = match.group(1)
        if "://" in target or target.startswith(("#", "mailto:")):
            continue
        target = unquote(target.split("#", 1)[0])
        if not target:
            continue
        resolved = bundle / target.lstrip("/") if target.startswith("/") else path.parent / target
        if not resolved.exists():
            report.add(rel, WARNING, "warn.link.broken", f"link target does not exist: {target}")


def check_metadata(report: Report, rel: Path, data: dict) -> None:
    missing = [key for key in RECOMMENDED_KEYS if not data.get(key)]
    if missing:
        report.add(
            rel,
            WARNING,
            "warn.metadata.recommended",
            f"recommended metadata missing: {', '.join(missing)}",
        )

    status = data.get("status", "stable")
    if status not in STATUS_VALUES:
        report.add(
            rel,
            WARNING,
            "warn.status.unknown",
            f"status is {status!r}, expected one of {', '.join(STATUS_VALUES)}",
        )

    stale_after = data.get("stale_after")
    if stale_after is not None:
        stale_date = as_date(stale_after)
        if stale_date is None:
            report.add(
                rel,
                WARNING,
                "warn.stale",
                f"stale_after is not an ISO date: {stale_after!r}",
            )
        elif datetime.date.today() >= stale_date:
            report.add(rel, WARNING, "warn.stale", f"content is stale as of {stale_date}")

    generated = data.get("generated")
    generated_by = generated.get("by") if isinstance(generated, dict) else None
    verified = as_verification_list(data.get("verified"))

    for label, actor in [("generated.by", generated_by)] + [
        ("verified[].by", entry.get("by")) for entry in verified
    ]:
        if actor is not None and not ACTOR.match(str(actor)):
            report.add(
                rel,
                WARNING,
                "warn.actor.convention",
                f"{label} is {actor!r}; expected human:<id>, process:<id>, or <producer>/<version>",
            )

    if generated_by and not str(generated_by).startswith("human:") and not verified:
        if status == "stable":
            report.add(
                rel,
                WARNING,
                "warn.unverified",
                "machine-generated content is stable with no verified entry",
            )


def as_date(value) -> datetime.date | None:
    if isinstance(value, datetime.datetime):
        return value.date()
    if isinstance(value, datetime.date):
        return value
    if isinstance(value, str) and ISO_DATE.match(value):
        try:
            return datetime.date.fromisoformat(value)
        except ValueError:
            return None
    return None


def as_verification_list(value) -> list[dict]:
    """OKF section 5.2: a bare mapping is a one-element list."""
    if isinstance(value, dict):
        return [value]
    if isinstance(value, list):
        return [entry for entry in value if isinstance(entry, dict)]
    return []


def check_concept(report: Report, path: Path, rel: Path, text: str) -> None:
    report.concepts += 1
    kind, frontmatter, body = split_frontmatter(text)

    if path.parent.name == ADR_DIRECTORY:
        check_adr(report, rel, path.name, body)

    if kind == "none":
        report.add(rel, OKF_ERROR, "okf.frontmatter.missing", "no YAML frontmatter block")
        return
    if kind == "unterminated":
        report.add(
            rel,
            OKF_ERROR,
            "okf.frontmatter.missing",
            "frontmatter block has no closing `---`",
        )
        return

    data = parse_frontmatter(report, rel, frontmatter)
    if data is None:
        return

    concept_type = data.get("type")
    if not isinstance(concept_type, str) or not concept_type.strip():
        report.add(rel, OKF_ERROR, "okf.type.missing", "frontmatter has no non-empty `type`")

    check_metadata(report, rel, data)


def check_adr(report: Report, rel: Path, name: str, body: str) -> None:
    if not ADR_FILENAME.match(name):
        report.add(
            rel,
            POLICY_ERROR,
            "policy.adr.filename",
            "ADR file name must be adr-NNNN-<kebab-case-slug>.md",
        )
    present = {title.casefold() for _, title in headings(body)}
    missing = [section for section in ADR_SECTIONS if section.casefold() not in present]
    if missing:
        report.add(
            rel,
            POLICY_ERROR,
            "policy.adr.sections",
            f"ADR is missing required section(s): {', '.join(missing)}",
        )


def check_index(report: Report, rel: Path, text: str, is_root: bool) -> dict | None:
    """OKF section 8. Returns the root index frontmatter when there is one."""
    kind, frontmatter, body = split_frontmatter(text)
    data = None
    if kind == "ok":
        if not is_root:
            report.add(
                rel,
                OKF_ERROR,
                "okf.index.frontmatter",
                "only a bundle-root index.md may carry frontmatter",
            )
        else:
            data = parse_frontmatter(report, rel, frontmatter)
    elif kind == "unterminated":
        report.add(
            rel,
            OKF_ERROR,
            "okf.index.frontmatter",
            "frontmatter block has no closing `---`",
        )

    stripped = without_code_fences(body)
    has_heading = any(HEADING.match(line) for line in stripped.splitlines())
    has_link_bullet = any(
        LIST_ITEM.match(line) and MARKDOWN_LINK.search(line) for line in stripped.splitlines()
    )
    if not has_heading or not has_link_bullet:
        report.add(
            rel,
            OKF_ERROR,
            "okf.index.structure",
            "index.md must list contents as section headings with link bullets",
        )
    return data


def check_log(report: Report, rel: Path, text: str) -> None:
    """OKF section 9. Only ever runs when log.md exists; it is never required."""
    _, _, body = split_frontmatter(text)
    found = headings(body)
    if found and found[0][0] == 1 and as_date(found[0][1]) is None:
        found = found[1:]  # a leading level-1 document title, not a date group
    if not found:
        report.add(
            rel,
            OKF_ERROR,
            "okf.log.structure",
            "log.md must group entries under ISO 8601 date headings",
        )
        return

    parsed = []
    for _, title in found:
        date = as_date(title)
        if date is None:
            report.add(
                rel,
                OKF_ERROR,
                "okf.log.date",
                f"log date heading is not an ISO 8601 YYYY-MM-DD date: {title!r}",
            )
        else:
            parsed.append(date)
    if parsed != sorted(parsed, reverse=True):
        report.add(rel, WARNING, "warn.log.order", "log entries are not newest first")


def check_bundle(bundle: Path, report: Report) -> None:
    root_index = bundle / "index.md"
    if not root_index.is_file():
        report.add(
            Path(bundle.name) / "index.md",
            POLICY_ERROR,
            "policy.index.missing",
            "the bundle root must contain index.md",
        )

    for path in sorted(bundle.rglob("*.md")):
        rel = path.relative_to(bundle.parent)
        text = path.read_text(encoding="utf-8")
        check_links(report, bundle, path, rel, text)

        if path.name == "index.md":
            data = check_index(report, rel, text, is_root=path == root_index)
            if path == root_index:
                check_okf_version(report, rel, data)
        elif path.name == "log.md":
            check_log(report, rel, text)
        else:
            check_concept(report, path, rel, text)


def check_okf_version(report: Report, rel: Path, data: dict | None) -> None:
    if data is None:
        report.add(
            rel,
            POLICY_ERROR,
            "policy.okf_version",
            'the bundle root must declare okf_version: "0.2"',
        )
        return
    version = data.get("okf_version")
    if version != "0.2":
        report.add(
            rel,
            POLICY_ERROR,
            "policy.okf_version",
            f'okf_version must be the string "0.2", found {version!r}',
        )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("bundle", help="path to the knowledge bundle root")
    args = parser.parse_args(argv)

    if yaml is None:
        print(
            "check_knowledge.py needs PyYAML to read frontmatter: python -m pip install pyyaml",
            file=sys.stderr,
        )
        return 2

    bundle = Path(args.bundle)
    if not bundle.is_dir():
        print(f"not a directory: {bundle}", file=sys.stderr)
        return 2

    report = Report()
    check_bundle(bundle, report)
    report.write(sys.stdout)
    return 1 if report.errors else 0


if __name__ == "__main__":
    sys.exit(main())
