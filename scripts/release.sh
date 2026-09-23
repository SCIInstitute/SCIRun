#!/bin/bash
set -euo pipefail

# Who may cut a release. Not real access control -- anyone with push rights
# to master could bypass this -- just a guard against running this by
# accident on the wrong machine/account.
RELEASE_ALLOWED_USERS=("dcwhite" "jessdtate")

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

if ! command -v gh >/dev/null 2>&1; then
    echo "ERROR: the gh CLI is required (to check who's releasing and to toggle branch protection)."
    exit 1
fi

GH_USER="$(gh api user --jq .login 2>/dev/null)" || {
    echo "ERROR: could not determine the authenticated GitHub user. Run 'gh auth login' first."
    exit 1
}

authorized=0
for allowed in "${RELEASE_ALLOWED_USERS[@]}"; do
    [[ "$GH_USER" == "$allowed" ]] && authorized=1
done
if [[ "$authorized" != "1" ]]; then
    echo "ERROR: GitHub user '$GH_USER' is not in the release allowlist (${RELEASE_ALLOWED_USERS[*]})."
    exit 1
fi

if [[ -n "$(git status --porcelain)" ]]; then
    echo "ERROR: working tree at $SRC_DIR is dirty. Commit or stash before releasing."
    exit 1
fi

if git rev-parse "$VERSION" >/dev/null 2>&1; then
    echo "ERROR: tag $VERSION already exists."
    exit 1
fi

# Branch protection on master must come down for the push below (it blocks
# direct pushes). Toggle the *same* protection rule's pattern away from
# "master" and back, rather than deleting/recreating it, so none of its
# settings can drift on restore.
REPO_NWO="$(gh repo view --json nameWithOwner -q .nameWithOwner)"
REPO_OWNER="${REPO_NWO%%/*}"
REPO_NAME="${REPO_NWO##*/}"

BP_RULE_ID="$(gh api graphql -f query='
query($owner: String!, $name: String!) {
  repository(owner: $owner, name: $name) {
    branchProtectionRules(first: 20) {
      nodes { id pattern }
    }
  }
}' -f owner="$REPO_OWNER" -f name="$REPO_NAME" \
    --jq '.data.repository.branchProtectionRules.nodes[] | select(.pattern=="master") | .id')"

if [[ -z "$BP_RULE_ID" ]]; then
    echo "ERROR: could not find a branch protection rule with pattern 'master' on $REPO_NWO."
    exit 1
fi

BP_DISABLED=0

set_branch_protection_pattern() {
    gh api graphql -f query='
mutation($id: ID!, $pattern: String!) {
  updateBranchProtectionRule(input: {branchProtectionRuleId: $id, pattern: $pattern}) {
    branchProtectionRule { pattern }
  }
}' -f id="$BP_RULE_ID" -f pattern="$1" >/dev/null
}

restore_branch_protection() {
    if [[ "$BP_DISABLED" == "1" ]]; then
        echo "Restoring branch protection on master..."
        set_branch_protection_pattern "master"
        BP_DISABLED=0
    fi
}
trap restore_branch_protection EXIT

git fetch origin master
git checkout master
git merge --ff-only origin/master

echo "$VERSION" > LATEST_TAG.txt
git add LATEST_TAG.txt
git commit -m "Tagged release $VERSION"
git tag -a "$VERSION" -m "Release $VERSION"

echo
echo "About to push to origin/master and push tag $VERSION, as $GH_USER."
read -r -p "Continue? [y/N] " confirm
if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
    echo "Aborted. Local commit and tag were created but not pushed."
    echo "To undo: git reset --hard origin/master && git tag -d $VERSION"
    exit 1
fi

echo "Temporarily disabling branch protection on master..."
set_branch_protection_pattern "master1"
BP_DISABLED=1

git push origin master
git push origin "$VERSION"

restore_branch_protection
