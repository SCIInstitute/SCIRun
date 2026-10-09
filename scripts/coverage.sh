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
# %c => continuous mode: counters are mmapped into the file as they change.
# Regression mode leaves via _Exit (Core::quickExit), which skips the atexit
# hook that would otherwise write them, so without %c those tests record
# nothing. Flushing by hand is no fix: each dylib has its own runtime copy.
export LLVM_PROFILE_FILE="$(cd "$COV_DIR" && pwd)/%c%p.profraw"

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

# The build dir holds only generated code (moc_*, factory *_Generated.cc): ~40k
# untestable lines that cut the line total by 4 points. Physical path because
# that's what CMake records; regex-escaped since it's spliced into the pattern.
BUILD_ABS="$(cd "$BUILD_DIR" && pwd -P | sed 's/[][\.*^$+?(){}|]/\\&/g')"
IGNORE="(Externals|/Testing/|googletest|/usr/|/Applications/|\\.framework/|^${BUILD_ABS}/)"

echo ">>> Coverage summary"
xcrun llvm-cov report "${OBJECTS[@]}" \
  -instr-profile="$COV_DIR/merged.profdata" \
  -ignore-filename-regex="$IGNORE" | tee "$COV_DIR/summary.txt"

# llvm-cov's "N functions have mismatched data" is mostly noise: an image that
# includes an inline function but never calls it records it under hash 0, and
# llvm-cov warns about that copy while still using the image that did call it.
# A non-zero hash is a function compiled differently in two images; those can
# lose counts, so list them.
xcrun llvm-cov report "${OBJECTS[@]}" \
  -instr-profile="$COV_DIR/merged.profdata" -dump 2>&1 \
  | sed -n "s/^hash-mismatch: No profile record found for '\(.*\)' with hash = \(0x[0-9a-f]*\).*/\2 \1/p" \
  > "$COV_DIR/mismatch-raw.txt" || true
grep -v '^0x0 ' "$COV_DIR/mismatch-raw.txt" | cut -d' ' -f2- \
  | xcrun llvm-cxxfilt -n | sort | uniq -c | sort -rn > "$COV_DIR/mismatched.txt" || true
echo ">>> hash mismatches: $(grep -c '^0x0 ' "$COV_DIR/mismatch-raw.txt") unused-copy (harmless)," \
  "$(wc -l < "$COV_DIR/mismatched.txt" | tr -d ' ') real: $COV_DIR/mismatched.txt"
rm -f "$COV_DIR/mismatch-raw.txt"

echo ">>> Generating HTML report"
xcrun llvm-cov show "${OBJECTS[@]}" \
  -instr-profile="$COV_DIR/merged.profdata" \
  -format=html -output-dir="$COV_DIR/html" \
  -ignore-filename-regex="$IGNORE" \
  -show-line-counts-or-regions >/dev/null

echo ""
echo "Summary: $COV_DIR/summary.txt"
echo "HTML:    $COV_DIR/html/index.html"
