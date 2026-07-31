#!/usr/bin/env bash
#
# Run the SCIRun regression suite and clean up the per-process QSettings stores
# it creates.
#
# In regression mode each test process isolates its QSettings under an app name
# "SCIRun5_regression_<pid>" (so concurrent processes don't race on the shared
# store). Those per-process stores would otherwise accumulate, so this wrapper
# removes them before and after the run. Cleanup is done here rather than in the
# app because on macOS cfprefsd owns the plist and recreates it if a live
# process deletes it; once the run is over the domains are dead and removal is
# reliable.
#
# Usage:
#   scripts/run-regression-tests.sh [build-dir] [-- <extra ctest args>]
#
# Examples:
#   scripts/run-regression-tests.sh                          # bin/SCIRun, default ctest args
#   scripts/run-regression-tests.sh bin/SCIRun -- -j3        # parallel
#   scripts/run-regression-tests.sh bin/SCIRun -- -j3 -R Renderer
#
set -uo pipefail

BUILD_DIR="bin/SCIRun"
if [[ $# -gt 0 && "$1" != "--" ]]; then
  BUILD_DIR="$1"; shift
fi
CTEST_ARGS=(-R "\.Test\.ExampleNetwork\." --output-on-failure)
if [[ "${1:-}" == "--" ]]; then
  shift; CTEST_ARGS=("$@")
fi

cleanup_regression_settings() {
  case "$(uname -s)" in
    Darwin)
      # cfprefsd may keep the dead domains cached briefly; ask it to drop them,
      # then remove any leftover files.
      for f in "$HOME"/Library/Preferences/com.sci-cibc-software.SCIRun5_regression_*.plist; do
        [[ -e "$f" ]] || continue
        local base; base="$(basename "$f" .plist)"
        defaults delete "$base" >/dev/null 2>&1 || true
        rm -f "$f" || true
      done
      ;;
    Linux)
      rm -f "$HOME"/.config/"SCI:CIBC Software"/SCIRun5_regression_*.conf 2>/dev/null || true
      ;;
    *)
      # Windows (Git Bash / MSYS): QSettings uses the registry; remove per-pid keys.
      if command -v reg >/dev/null 2>&1; then
        reg query "HKCU\\Software\\SCI:CIBC Software" 2>/dev/null \
          | grep -i "SCIRun5_regression_" \
          | while read -r key; do reg delete "$key" /f >/dev/null 2>&1 || true; done
      fi
      ;;
  esac
}

echo ">>> Cleaning stale regression settings"
cleanup_regression_settings

echo ">>> Running: (cd $BUILD_DIR && ctest ${CTEST_ARGS[*]})"
( cd "$BUILD_DIR" && ctest "${CTEST_ARGS[@]}" )
rc=$?

echo ">>> Cleaning up regression settings"
cleanup_regression_settings

exit $rc
