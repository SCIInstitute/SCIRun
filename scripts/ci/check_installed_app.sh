#!/usr/bin/env bash
# Check that an installed SCIRun.app is self-contained: nothing it loads, or
# could load, lives outside the bundle or the OS. On a CI runner the build tree
# is still present, so a leaked build path resolves fine there and only breaks
# on a user's machine -- hence checking paths rather than just "does it start".
#
# Usage: check_installed_app.sh /Applications/SCIRun.app
set -euo pipefail

APP=${1:?usage: $0 /path/to/SCIRun.app}
APP=${APP%/}
EXE="$APP/Contents/MacOS/SCIRun"
[ -x "$EXE" ] || { echo "::error::$EXE not found or not executable"; exit 1; }

outside() { grep -v -e "^$APP/" -e '^/System/' -e '^/usr/lib/' -e '^@' || true; }

# Static: every load command in every Mach-O in the bundle. Warn-only until
# #2805 (bundled python3.13 links to the build tree's Python) is fixed.
static=$(
  find "$APP/Contents" -type f -print0 | while IFS= read -r -d '' f; do
    file -b "$f" | grep -q Mach-O || continue
    id=$(otool -D -X "$f" 2>/dev/null | head -1)
    otool -L -X "$f" | awk '{print $1}' | grep -vxF -- "${id:-//}" | outside |
      sed "s|^|${f#"$APP"/} -> |"
  done
)
if [ -n "$static" ]; then
  while IFS= read -r line; do echo "::warning title=Non-bundled dylib reference::$line"; done <<< "$static"
fi

# Dynamic: what dyld actually resolves when the bundled binary runs. --help
# goes to ConsoleApplication, so no window server is needed.
loaded=$(DYLD_PRINT_LIBRARIES=1 "$EXE" --help 2>&1 >/dev/null |
  sed -n 's/^dyld\[[0-9]*\]: <[0-9A-F-]*> //p')
"$EXE" --help >/dev/null

inbundle=$(grep -c "^$APP/" <<< "$loaded" || true)
leaked=$(outside <<< "$loaded")
echo "dyld loaded $(wc -l <<< "$loaded" | tr -d ' ') images, $inbundle from the bundle"
if [ -n "$leaked" ]; then
  while IFS= read -r line; do echo "::error title=Loaded from outside the bundle::$line"; done <<< "$leaked"
  exit 1
fi
# An empty list means DYLD_PRINT_LIBRARIES was ignored, not that all is well.
[ "$inbundle" -gt 0 ] || { echo "::error::dyld reported no bundle libraries"; exit 1; }
