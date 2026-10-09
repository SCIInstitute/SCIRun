#!/usr/bin/env bash
# Throwaway (#2732): loop setuptdcs_colin27_patchelc on a CI mac runner, once per workaround
# variant, stopping a variant after MAX_HITS failing passes or MAX_PASSES passes.
#
# Each pass runs the windowed net in the foreground and two headless copies beside it: the
# nightly runs ctest -j3 on a 3-core runner, and the gpu RESOURCE_LOCK keeps the other two
# tests off ViewScene, so this is the load the crash has been seen under.
#
# Variants are runtime switches read by the branch's code (SCIRUN_2732_*), so one build
# covers them all. "base" sets none.
#
# usage: ci-loop-2732.sh <max_passes> <max_hits> <outdir> <variant>...
set -u

MAX_PASSES=${1:-20}
MAX_HITS=${2:-5}
OUT=${3:-loop-out}
shift 3
VARIANTS=("$@")
[ "${#VARIANTS[@]}" -gt 0 ] || VARIANTS=(base)

BIN="$GITHUB_WORKSPACE/bin/SCIRun/SCIRun_test"
NET="$GITHUB_WORKSPACE/src/ExampleNets/regression/Modules/setuptdcs_colin27_patchelc.srn5"
DATA="$GITHUB_WORKSPACE/SCIRunTestData"
ARGS=(-E "$NET" --no_splash --regression 180 -d "$DATA")
ASAN_BASE="halt_on_error=1:abort_on_error=0:detect_leaks=0"

mkdir -p "$OUT"
SUMMARY="$OUT/summary.txt"
: > "$SUMMARY"
TABLE="| variant | passes | hits |"$'\n'"|---|---|---|"

# 12 is normal for the headless copies: -x on a GUI build errors on the ScreenshotData ports.
bad() { [ "$1" -ne 0 ] && [ "$1" -ne 12 ]; }

for v in "${VARIANTS[@]}"; do
  unset SCIRUN_2732_NOFLAGS SCIRUN_2732_NORESIZE SCIRUN_2732_NOFLOAT
  case "$v" in
    base) ;;
    noflags) export SCIRUN_2732_NOFLAGS=1 ;;
    noresize) export SCIRUN_2732_NORESIZE=1 ;;
    nofloat) export SCIRUN_2732_NOFLOAT=1 ;;
    *) echo "unknown variant $v"; continue ;;
  esac

  VOUT="$OUT/$v"
  mkdir -p "$VOUT"
  i=0
  hits=0
  while [ "$i" -lt "$MAX_PASSES" ] && [ "$hits" -lt "$MAX_HITS" ]; do
    i=$((i + 1))

    ASAN_OPTIONS="$ASAN_BASE:log_path=$VOUT/asan-$i-bg1" "$BIN" -x "${ARGS[@]}" > "$VOUT/bg1-$i.log" 2>&1 &
    bg1=$!
    ASAN_OPTIONS="$ASAN_BASE:log_path=$VOUT/asan-$i-bg2" "$BIN" -x "${ARGS[@]}" > "$VOUT/bg2-$i.log" 2>&1 &
    bg2=$!

    start=$SECONDS
    ASAN_OPTIONS="$ASAN_BASE:log_path=$VOUT/asan-$i-gui" "$BIN" "${ARGS[@]}" > "$VOUT/gui-$i.log" 2>&1
    rc=$?
    wait "$bg1"; rc1=$?
    wait "$bg2"; rc2=$?

    asan=$(ls "$VOUT"/asan-"$i"-* 2>/dev/null | wc -l | tr -d ' ')
    line="variant=$v pass=$i gui_rc=$rc bg1_rc=$rc1 bg2_rc=$rc2 asan_reports=$asan secs=$((SECONDS - start))"
    echo "$line" | tee -a "$SUMMARY"

    if [ "$rc" -ne 0 ] || bad "$rc1" || bad "$rc2" || [ "$asan" -ne 0 ]; then
      hits=$((hits + 1))
    else
      rm -f "$VOUT/gui-$i.log" "$VOUT/bg1-$i.log" "$VOUT/bg2-$i.log"
    fi

    # Regression mode keeps QSettings per pid; don't let them pile up.
    rm -f "$HOME"/Library/Preferences/com.sci-cibc-software.SCIRun5_regression_*.plist 2>/dev/null
  done
  echo "variant=$v done: $i passes, $hits hits" | tee -a "$SUMMARY"
  TABLE+=$'\n'"| $v | $i | $hits |"
done

{
  echo "### #2732 workaround variants"
  echo "$TABLE"
} | tee -a "$SUMMARY" >> "${GITHUB_STEP_SUMMARY:-/dev/null}"
exit 0
