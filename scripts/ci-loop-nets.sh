#!/usr/bin/env bash
# Throwaway (#2700): loop regression nets on a CI mac runner under ASan, one net at a time,
# stopping a net after MAX_HITS failing passes or MAX_PASSES passes.
#
# Each pass runs the windowed net with two headless copies beside it, the load the nightly's
# ctest -j3 puts on a 3-core runner. A hit is a nonzero windowed exit, a signal from a headless
# copy (they exit 10-12 by design: -x errors on the ScreenshotData ports), or any ASan report.
#
# usage: ci-loop-nets.sh <max_passes> <max_hits> <outdir> <net.srn5>...
set -u

MAX_PASSES=${1:-8}
MAX_HITS=${2:-3}
OUT=${3:-loop-out}
shift 3

BIN="$GITHUB_WORKSPACE/bin/SCIRun/SCIRun_test"
DATA="$GITHUB_WORKSPACE/SCIRunTestData"
ASAN_BASE="halt_on_error=1:abort_on_error=0:detect_leaks=0"

mkdir -p "$OUT"
SUMMARY="$OUT/summary.txt"
: > "$SUMMARY"
TABLE="| net | passes | hits | passes that rendered | top ASan frames |"$'\n'"|---|---|---|---|---|"

bad() { [ "$1" -ge 128 ]; }

for net in "$@"; do
  name=$(basename "$net" .srn5)
  ARGS=(-E "$GITHUB_WORKSPACE/$net" --no_splash --regression 180 -d "$DATA")
  NOUT="$OUT/$name"
  mkdir -p "$NOUT"
  i=0
  hits=0
  while [ "$i" -lt "$MAX_PASSES" ] && [ "$hits" -lt "$MAX_HITS" ]; do
    i=$((i + 1))

    ASAN_OPTIONS="$ASAN_BASE:log_path=$NOUT/asan-$i-bg1" "$BIN" -x "${ARGS[@]}" > "$NOUT/bg1-$i.log" 2>&1 &
    bg1=$!
    ASAN_OPTIONS="$ASAN_BASE:log_path=$NOUT/asan-$i-bg2" "$BIN" -x "${ARGS[@]}" > "$NOUT/bg2-$i.log" 2>&1 &
    bg2=$!

    start=$SECONDS
    ASAN_OPTIONS="$ASAN_BASE:log_path=$NOUT/asan-$i-gui" "$BIN" "${ARGS[@]}" > "$NOUT/gui-$i.log" 2>&1
    rc=$?
    wait "$bg1"; rc1=$?
    wait "$bg2"; rc2=$?

    asan=$(ls "$NOUT"/asan-"$i"-* 2>/dev/null | wc -l | tr -d ' ')
    shots=$(grep -c "PROBE2732 screenshot" "$NOUT/gui-$i.log")
    line="net=$name pass=$i gui_rc=$rc bg1_rc=$rc1 bg2_rc=$rc2 asan_reports=$asan screenshots=$shots secs=$((SECONDS - start))"
    echo "$line" | tee -a "$SUMMARY"

    if [ "$rc" -ne 0 ] || bad "$rc1" || bad "$rc2" || [ "$asan" -ne 0 ]; then
      hits=$((hits + 1))
    else
      rm -f "$NOUT/gui-$i.log" "$NOUT/bg1-$i.log" "$NOUT/bg2-$i.log"
    fi
    rm -f "$HOME"/Library/Preferences/com.sci-cibc-software.SCIRun5_regression_*.plist 2>/dev/null
  done

  rendered=$(grep -c "net=$name pass=.* screenshots=[1-9]" "$SUMMARY")
  # Frames #0-#2 of each report, symbol names only, most common first.
  frames=$(for f in "$NOUT"/asan-*; do
      [ -f "$f" ] && grep -E "^ +#[0-2] " "$f" | sed -E 's/^ +#[0-9]+ 0x[0-9a-f]+ (in )?//; s/[ (].*//' | head -3 | paste -sd'<' -
    done | sort | uniq -c | sort -rn | head -2 | sed -E 's/^ +//' | paste -sd';' -)
  echo "net=$name done: $i passes, $hits hits, $rendered rendered, frames: ${frames:-none}" | tee -a "$SUMMARY"
  TABLE+=$'\n'"| $name | $i | $hits | $rendered | ${frames:-none} |"
done

{
  echo "### #2700 net loop"
  echo "$TABLE"
} | tee -a "$SUMMARY" >> "${GITHUB_STEP_SUMMARY:-/dev/null}"
exit 0
