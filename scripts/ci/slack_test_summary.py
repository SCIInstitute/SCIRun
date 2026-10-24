#!/usr/bin/env python3
"""Build a Slack Block Kit payload summarising a nightly run's test results.

Reads the `test-results-*` artifacts downloaded from a workflow run and writes
Slack JSON to stdout. Used by .github/workflows/nightly-slack.yml; kept as a
standalone script because `workflow_run` triggers cannot be exercised from a
branch, so this is the only part of that workflow that can be tested at all:

    gh run download <run-id> --dir artifacts --pattern 'test-results-*'
    WF_NAME=regression-tests WF_CONCLUSION=success \\
      python3 scripts/ci/slack_test_summary.py artifacts

Artifact layout is NOT uniform across platforms. The Unix upload step mixes an
absolute path (/tmp/unit-test-results.txt) with workspace-relative ones, so
upload-artifact roots the archive at / and the files land under
`test-results-Linux-headless/home/runner/work/...`; the all-relative Windows
paths land directly under the artifact directory. Hence rglob, not glob.

Coverage is optional: pass the directories `coverage-report-*` artifacts were
downloaded into (this run's, then the previous nightly's for the trend):

    gh run download <run-id> --dir coverage --pattern 'coverage-report-*'
    python3 scripts/ci/slack_test_summary.py artifacts coverage coverage-prev
"""

import json
import os
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

# Failing test names per report, before collapsing into "...and N more". Slack
# truncates a context element at 3000 chars and the names here run ~90.
MAX_NAMES = 5

CTEST_SUMMARY = re.compile(
    r"(\d+)% tests passed, (\d+) tests failed out of (\d+)")

# ElementTree's exposure is entity-expansion DoS -- billion laughs and quadratic
# blowup. Both need a DOCTYPE, and ctest's junit output never has one, so
# rejecting it removes the vector without a defusedxml dependency, which this
# script cannot take (see the module docstring: it must run on a stock
# interpreter). External entities are not a concern; CPython's expat does not
# fetch them.
DOCTYPE = re.compile(rb"<!DOCTYPE", re.IGNORECASE)


# Uncovered-directory listing length. Ranked by missed lines, not percentage:
# a 0% directory with 20 lines matters less than a 40% one with 10k.
MAX_COV_DIRS = 5

# llvm-cov report row: filename, then regions/missed/cover, functions/missed/
# executed, lines/missed/cover, branches/missed/cover. A cover is "-" for 0/0.
COV_ROW = re.compile(
    r"^(?P<name>\S.*?)\s+" + r"\s+".join([r"(\d+)\s+(\d+)\s+(\S+)"] * 4)
    + r"\s*$")


class Coverage:
    """Totals and per-directory line counts from one llvm-cov summary.txt."""

    def __init__(self, label):
        self.label = label
        self.totals = {}  # metric -> (count, missed)
        self.dirs = {}    # directory -> [lines, missed]

    def pct(self, metric):
        count, missed = self.totals.get(metric, (0, 0))
        return 100.0 * (count - missed) / count if count else None


def coverage_dir_of(name):
    # Rows are relative to llvm-cov's common prefix, e.g.
    # "SCIRun/src/Core/Datatypes/Field.cc"; group two levels below src/.
    # None outside src/: bin/ moc and factory output and Qt headers would
    # otherwise top the listing with code nobody can write tests for.
    parts = name.split("/")
    if "src" not in parts:
        return None
    parts = parts[parts.index("src") + 1:]
    return "/".join(parts[:2]) if len(parts) > 2 else "/".join(parts[:-1]) or "."


def parse_coverage(path, label):
    cov = Coverage(label)
    for line in path.read_text(errors="replace").splitlines():
        m = COV_ROW.match(line)
        if m is None:
            continue
        g = m.groups()
        nums = {"regions": (int(g[1]), int(g[2])),
                "functions": (int(g[4]), int(g[5])),
                "lines": (int(g[7]), int(g[8])),
                "branches": (int(g[10]), int(g[11]))}
        if g[0] == "TOTAL":
            cov.totals = nums
            continue
        dirname = coverage_dir_of(g[0])
        if dirname is None:
            continue
        d = cov.dirs.setdefault(dirname, [0, 0])
        d[0] += nums["lines"][0]
        d[1] += nums["lines"][1]
    return cov if cov.totals else None


def collect_coverage(coverage_dir):
    if coverage_dir is None or not coverage_dir.is_dir():
        return None
    for summary in sorted(coverage_dir.rglob("summary.txt")):
        # Artifact dir is coverage-report-<os>; gh drops it for a lone match.
        label = next((p.name.removeprefix("coverage-report-")
                      for p in summary.parents
                      if p.name.startswith("coverage-report-")), "")
        cov = parse_coverage(summary, label)
        if cov is not None:
            return cov
    return None


def coverage_blocks(cov, prev, url):
    if cov is None:
        return [{"type": "context",
                 "elements": [{"type": "mrkdwn",
                               "text": ":warning: No coverage summary found "
                                       "in this run's artifacts."}]}]

    def trend(metric):
        now = cov.pct(metric)
        before = prev.pct(metric) if prev else None
        if now is None or before is None:
            return ""
        delta = now - before
        if abs(delta) < 0.005:
            return " (±0)"
        return f" ({'▲' if delta > 0 else '▼'}{abs(delta):.2f})"

    def fmt(metric):
        p = cov.pct(metric)
        return "–" if p is None else f"{p:.2f}%{trend(metric)}"

    where = f" ({cov.label})" if cov.label else ""
    text = (f":bar_chart: *Coverage*{where}: *lines {fmt('lines')}* · "
            f"functions {fmt('functions')} · regions {fmt('regions')} · "
            f"branches {fmt('branches')}")
    if url:
        text += f" — <{url}|HTML report>"
    blocks = [{"type": "section", "text": {"type": "mrkdwn", "text": text}}]

    worst = sorted(((missed, lines, d) for d, (lines, missed)
                    in cov.dirs.items() if missed),
                   reverse=True)[:MAX_COV_DIRS]
    if worst:
        listing = " · ".join(
            f"`{d}` {missed:,} ({100.0 * (lines - missed) / lines:.0f}%)"
            for missed, lines, d in worst)
        blocks.append({"type": "context",
                       "elements": [{"type": "mrkdwn",
                                     "text": f"*Most uncovered lines:* "
                                             f"{listing}"}]})
    return blocks


class Report:
    """One junit file: a (job, kind) pair such as Linux-headless / regression."""

    def __init__(self, label, kind):
        self.label = label
        self.kind = kind
        self.total = 0
        self.failed = 0
        self.skipped = 0
        self.names = []

    @property
    def passed(self):
        return self.total - self.failed - self.skipped


def parse_junit(path, label, kind):
    raw = path.read_bytes()
    if DOCTYPE.search(raw):
        raise ET.ParseError("DOCTYPE is not allowed in test output")
    # No DOCTYPE, so no entity declarations can be present (B314 above).
    root = ET.fromstring(raw)  # nosec B314  # noqa: S314
    r = Report(label, kind)
    # ctest's own attributes are authoritative; `disabled` tests are counted in
    # `tests` here but excluded from ctest's console "out of N", which is why
    # the two disagree by exactly the disabled count.
    r.total = int(root.get("tests", 0))
    r.failed = int(root.get("failures", 0))
    r.skipped = int(root.get("disabled", 0)) + int(root.get("skipped", 0))
    r.names = [tc.get("name") for tc in root.iter("testcase")
               if tc.get("status") == "fail" or tc.find("failure") is not None]
    return r


def parse_ctest_text(path, label, kind):
    """Fallback for a run whose junit file is missing (ctest died early)."""
    m = None
    for m in CTEST_SUMMARY.finditer(path.read_text(errors="replace")):
        pass
    if m is None:
        return None
    r = Report(label, kind)
    r.failed = int(m.group(2))
    r.total = int(m.group(3))
    return r


def collect(artifacts_dir):
    reports = []
    for art in sorted(p for p in artifacts_dir.iterdir() if p.is_dir()):
        label = art.name.removeprefix("test-results-")
        for kind in ("unit", "regression"):
            junit = next(art.rglob(f"junit-{kind}.xml"), None)
            if junit is not None:
                try:
                    reports.append(parse_junit(junit, label, kind))
                    continue
                except ET.ParseError as exc:
                    print(f"warning: {junit}: {exc}", file=sys.stderr)
            text = next(art.rglob(f"{kind}-test-results.txt"), None)
            if text is not None:
                fallback = parse_ctest_text(text, label, kind)
                if fallback is not None:
                    reports.append(fallback)
    return reports


def emoji_for(conclusion, any_failed, failed_jobs):
    # Honour the real conclusion, but downgrade a green job to yellow when its
    # continue-on-error test steps or jobs actually failed underneath it.
    base = {
        "success": ":large_green_circle:",
        "failure": ":red_circle:",
        "cancelled": ":black_circle:",
    }.get(conclusion, ":white_circle:")
    if conclusion == "success" and (any_failed or failed_jobs):
        return ":large_yellow_circle:"
    return base


def build(reports, env, coverage=None, coverage_prev=None):
    name = env.get("WF_NAME", "workflow")
    conclusion = env.get("WF_CONCLUSION", "unknown")
    url = env.get("WF_URL", "")
    sha = env.get("WF_SHA", "")[:9]
    branch = env.get("WF_BRANCH", "")

    # Jobs that failed under continue-on-error, so the run's own conclusion is
    # still green. Set by nightly-slack.yml.
    failed_jobs = env.get("WF_FAILED_JOBS", "").strip()

    # Release URL, set by nightly-slack.yml only when this run's own installers
    # are the ones on the nightly tag. Empty means publish-nightly did not run
    # or did not win the tag, so linking would point at someone else's build.
    installer_url = env.get("WF_INSTALLER_URL", "").strip()

    any_failed = any(r.failed for r in reports)
    header = (f"{emoji_for(conclusion, any_failed, failed_jobs)} "
              f"*{name}* nightly — {conclusion}")

    blocks = [{"type": "section",
               "text": {"type": "mrkdwn", "text": header}}]

    # Only on a green run: there the failures are hidden, which is the whole
    # point. On a red one they are the stated cause, not "non-blocking".
    # Before the fields so the block-count trim below cannot drop it.
    if failed_jobs and conclusion == "success":
        blocks.append({
            "type": "context",
            "elements": [{"type": "mrkdwn",
                          "text": f"*failed, non-blocking:* {failed_jobs}"}],
        })

    # Also above the fields, for the same reason: this is the one line a reader
    # might actually want to click, so the trim must not be able to reach it.
    if installer_url:
        blocks.append({
            "type": "context",
            "elements": [{"type": "mrkdwn",
                          "text": f":package: <{installer_url}|Installers from "
                                  "this run> — unsigned, see #1663"}],
        })

    # Also above the fields, so the trim below cannot reach it. Only the
    # workflow that runs mac-coverage gets the block, missing or not.
    if coverage is not None or env.get("WF_NAME") == "regression-tests":
        blocks.extend(coverage_blocks(
            coverage, coverage_prev, env.get("WF_COVERAGE_URL", "").strip()))

    # One field per report, two columns. Slack caps a section at 10 fields, so
    # chunk rather than assume the matrix stays small.
    fields = []
    for r in reports:
        mark = "❌" if r.failed else "✅"
        fields.append({
            "type": "mrkdwn",
            "text": (f"*{r.label}* · {r.kind}\n"
                     f"{mark} {r.failed} failed / {r.total}"
                     + (f" · {r.skipped} skipped" if r.skipped else "")),
        })
    for i in range(0, len(fields), 10):
        blocks.append({"type": "section", "fields": fields[i:i + 10]})

    for r in reports:
        if not r.names:
            continue
        shown = r.names[:MAX_NAMES]
        more = len(r.names) - len(shown)
        listing = "\n".join(f"• {n}" for n in shown)
        if more:
            listing += f"\n• …and {more} more"
        blocks.append({
            "type": "context",
            "elements": [{"type": "mrkdwn",
                          "text": f"*{r.label} {r.kind}*\n{listing}"}],
        })

    footer = f"<{url}|View run>"
    if sha:
        footer += f" · `{sha}`"
    if branch:
        footer += f" ({branch})"
    blocks.append({"type": "context",
                   "elements": [{"type": "mrkdwn", "text": footer}]})

    # Slack hard-caps a message at 50 blocks; drop failure listings from the
    # middle rather than letting the whole post 400 out.
    if len(blocks) > 50:
        blocks = blocks[:49] + blocks[-1:]

    # `text` is the notification/fallback line, not shown in-channel when
    # blocks are present.
    return {"text": f"{name} nightly — {conclusion}", "blocks": blocks}


def main():
    args = sys.argv[1:] + [None] * 3
    artifacts_dir = Path(args[0] or "artifacts")
    reports = collect(artifacts_dir) if artifacts_dir.is_dir() else []
    coverage = collect_coverage(Path(args[1] or "coverage"))
    coverage_prev = collect_coverage(Path(args[2] or "coverage-prev"))
    json.dump(build(reports, os.environ, coverage, coverage_prev),
              sys.stdout, indent=2)
    sys.stdout.write("\n")


if __name__ == "__main__":
    main()
