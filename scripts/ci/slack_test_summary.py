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
    root = ET.parse(path).getroot()
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


def build(reports, env):
    name = env.get("WF_NAME", "workflow")
    conclusion = env.get("WF_CONCLUSION", "unknown")
    url = env.get("WF_URL", "")
    sha = env.get("WF_SHA", "")[:9]
    branch = env.get("WF_BRANCH", "")

    # Jobs that failed under continue-on-error, so the run's own conclusion is
    # still green. Set by nightly-slack.yml.
    failed_jobs = env.get("WF_FAILED_JOBS", "").strip()

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
    artifacts_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "artifacts")
    reports = collect(artifacts_dir) if artifacts_dir.is_dir() else []
    json.dump(build(reports, os.environ), sys.stdout, indent=2)
    sys.stdout.write("\n")


if __name__ == "__main__":
    main()
