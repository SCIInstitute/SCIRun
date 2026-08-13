#!/usr/bin/env bash
#
# Clang source-based code coverage for SCIRun.
#
# Prerequisite: configure and build with coverage instrumentation:
#   ./build.sh -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
# (a Debug build gives the most accurate line/region mapping)
#
# Usage:
#   scripts/coverage.sh [build-dir] [-- <extra ctest args>]
#
# Examples:
#   scripts/coverage.sh                              # all tests, build dir bin/SCIRun
#   scripts/coverage.sh bin/SCIRun -- -j3            # parallel
#   scripts/coverage.sh bin/SCIRun -- -R "\.Test\.ExampleNetwork\."   # regression only
#
# Output:
#   <build-dir>/coverage/summary.txt     line/region/branch summary
#   <build-dir>/coverage/html/index.html browsable HTML report
#
set -euo pipefail

BUILD_DIR="bin/SCIRun"
CTEST_ARGS=()
# First non-flag arg (before --) is the build dir; everything after -- is ctest args.
if [[ $# -gt 0 && "$1" != "--" ]]; then
  BUILD_DIR="$1"; shift
fi
if [[ "${1:-}" == "--" ]]; then
  shift; CTEST_ARGS=("$@")
fi

if [[ ! -x "$BUILD_DIR/SCIRun_test" ]]; then
  echo "error: $BUILD_DIR/SCIRun_test not found. Build with -DENABLE_COVERAGE=ON first." >&2
  exit 1
fi

COV_DIR="$BUILD_DIR/coverage"
mkdir -p "$COV_DIR"
rm -f "$COV_DIR"/*.profraw "$COV_DIR/merged.profdata"

# %p => one profraw per process. Each regression test is its own process, so
# this avoids the processes clobbering a single shared profile file.
export LLVM_PROFILE_FILE="$(cd "$COV_DIR" && pwd)/%p.profraw"

echo ">>> Running tests (LLVM_PROFILE_FILE=$LLVM_PROFILE_FILE)"
( cd "$BUILD_DIR" && ctest --output-on-failure "${CTEST_ARGS[@]}" ) || \
  echo ">>> (some tests failed; coverage still collected)"

shopt -s nullglob
PROFRAWS=("$COV_DIR"/*.profraw)
if [[ ${#PROFRAWS[@]} -eq 0 ]]; then
  echo "error: no .profraw files produced. Was the build instrumented (-DENABLE_COVERAGE=ON)?" >&2
  exit 1
fi
echo ">>> Merging ${#PROFRAWS[@]} profile(s)"
xcrun llvm-profdata merge -sparse "${PROFRAWS[@]}" -o "$COV_DIR/merged.profdata"

# llvm-cov needs every instrumented object: the test binary plus all SCIRun
# shared libraries it loads.
OBJECTS=(-object "$BUILD_DIR/SCIRun_test")
for lib in "$BUILD_DIR"/lib/*.dylib; do
  OBJECTS+=(-object "$lib")
done

IGNORE='(Externals|/Testing/|googletest|/usr/|/Applications/)'

echo ">>> Coverage summary"
xcrun llvm-cov report "${OBJECTS[@]}" \
  -instr-profile="$COV_DIR/merged.profdata" \
  -ignore-filename-regex="$IGNORE" | tee "$COV_DIR/summary.txt"

echo ">>> Generating HTML report"
xcrun llvm-cov show "${OBJECTS[@]}" \
  -instr-profile="$COV_DIR/merged.profdata" \
  -format=html -output-dir="$COV_DIR/html" \
  -ignore-filename-regex="$IGNORE" \
  -show-line-counts-or-regions >/dev/null

echo ""
echo "Summary: $COV_DIR/summary.txt"
echo "HTML:    $COV_DIR/html/index.html"
