#!/usr/bin/env bash
# Throwaway (#2732): loop setuptdcs_colin27_patchelc on a CI mac runner until it has
# collected MAX_HITS failures or MINUTES have passed.
#
# Each iteration runs the windowed net in the foreground and two headless copies beside
# it: the nightly runs ctest -j3 on a 3-core runner, and the gpu RESOURCE_LOCK keeps the
# other two tests off ViewScene, so this is the load the crash has been seen under.
#
# usage: ci-loop-2732.sh <minutes> <max_hits> <outdir>
set -u

MINUTES=${1:-240}
MAX_HITS=${2:-5}
OUT=${3:-loop-out}

BIN="$GITHUB_WORKSPACE/bin/SCIRun/SCIRun_test"
NET="$GITHUB_WORKSPACE/src/ExampleNets/regression/Modules/setuptdcs_colin27_patchelc.srn5"
DATA="$GITHUB_WORKSPACE/SCIRunTestData"
ARGS=(-E "$NET" --no_splash --regression 180 -d "$DATA")
ASAN_BASE="halt_on_error=1:abort_on_error=0:detect_leaks=0"

mkdir -p "$OUT"
SUMMARY="$OUT/summary.txt"
: > "$SUMMARY"

end=$((SECONDS + MINUTES * 60))
i=0
hits=0
while [ "$SECONDS" -lt "$end" ] && [ "$hits" -lt "$MAX_HITS" ]; do
  i=$((i + 1))

  ASAN_OPTIONS="$ASAN_BASE:log_path=$OUT/asan-$i-bg1" "$BIN" -x "${ARGS[@]}" > "$OUT/bg1-$i.log" 2>&1 &
  bg1=$!
  ASAN_OPTIONS="$ASAN_BASE:log_path=$OUT/asan-$i-bg2" "$BIN" -x "${ARGS[@]}" > "$OUT/bg2-$i.log" 2>&1 &
  bg2=$!

  start=$SECONDS
  ASAN_OPTIONS="$ASAN_BASE:log_path=$OUT/asan-$i-gui" "$BIN" "${ARGS[@]}" > "$OUT/gui-$i.log" 2>&1
  rc=$?
  wait "$bg1"; rc1=$?
  wait "$bg2"; rc2=$?

  asan=$(ls "$OUT"/asan-"$i"-* 2>/dev/null | wc -l | tr -d ' ')
  line="iter=$i gui_rc=$rc bg1_rc=$rc1 bg2_rc=$rc2 asan_reports=$asan secs=$((SECONDS - start)) elapsed=$SECONDS"
  echo "$line" | tee -a "$SUMMARY"

  # 12 is normal for the headless copies: -x on a GUI build errors on the ScreenshotData ports.
  bad() { [ "$1" -ne 0 ] && [ "$1" -ne 12 ]; }
  if [ "$rc" -ne 0 ] || bad "$rc1" || bad "$rc2" || [ "$asan" -ne 0 ]; then
    hits=$((hits + 1))
    echo "::warning::hit $hits at iteration $i ($line)"
  else
    rm -f "$OUT/gui-$i.log" "$OUT/bg1-$i.log" "$OUT/bg2-$i.log"
  fi

  # Regression mode keeps QSettings per pid; don't let hundreds of them pile up.
  rm -f "$HOME"/Library/Preferences/com.sci-cibc-software.SCIRun5_regression_*.plist 2>/dev/null
done

echo "done: $i iterations, $hits hits, ${SECONDS}s" | tee -a "$SUMMARY"
{
  echo "### #2732 loop: $i iterations, $hits hits"
  echo '```'
  grep -Ev "gui_rc=0 bg1_rc=(0|12) bg2_rc=(0|12) asan_reports=0" "$SUMMARY" | tail -40
  echo '```'
} >> "${GITHUB_STEP_SUMMARY:-/dev/null}"
exit 0
