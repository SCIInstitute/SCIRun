#!/bin/bash
set -euo pipefail

MINPARAMS=1

if [ $# -lt "$MINPARAMS" ]
then
  echo
  echo "Called with a single parameter of the form \"alpha.XX\" or \"beta.A\""
  echo
  exit 0
fi

VERSION="v5.0-$1"
echo "$VERSION"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR/../src"

cd "$SRC_DIR"

if [[ -n "$(git status --porcelain)" ]]; then
    echo "ERROR: working tree at $SRC_DIR is dirty. Commit or stash before releasing."
    exit 1
fi

if git rev-parse "$VERSION" >/dev/null 2>&1; then
    echo "ERROR: tag $VERSION already exists."
    exit 1
fi

git fetch origin master
git checkout master
git merge --ff-only origin/master

echo "$VERSION" > LATEST_TAG.txt
git add LATEST_TAG.txt
git commit -m "Tagged release $VERSION"
git tag -a "$VERSION" -m "Release $VERSION"

echo
echo "About to push to origin/master and push tag $VERSION."
read -r -p "Continue? [y/N] " confirm
if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
    echo "Aborted. Local commit and tag were created but not pushed."
    echo "To undo: git reset --hard origin/master && git tag -d $VERSION"
    exit 1
fi

git push origin master
git push origin "$VERSION"
