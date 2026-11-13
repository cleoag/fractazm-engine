#!/bin/sh
# Puts the source of one commit into a checkout of the public repository as a
# single new commit and tags it. The public repository has no history of this
# one: each released version of the application adds one commit.
#
#   scripts/publish_source.sh <commit> <tag> <checkout of the public repository>
#
# Left out of the published tree: comparison/results (dated records of test
# runs, with paths and device names of the machines they ran on) and CLAUDE.md
# (it concerns the private working repository only).
#
# The script commits and tags in the checkout and stops there: pushing is a
# separate step, so that the tree can be looked at first.
set -eu

if [ $# -ne 3 ]; then
    sed -n '2,13p' "$0" | sed 's/^# \{0,1\}//'
    exit 2
fi
commit=$(git -C "$(dirname "$0")/.." rev-parse --verify "$1^{commit}")
tag=$2
public=$3
root=$(git -C "$(dirname "$0")/.." rev-parse --show-toplevel)

[ -d "$public/.git" ] || { echo "not a Git checkout: $public" >&2; exit 1; }
[ -z "$(git -C "$public" status --porcelain)" ] || { echo "the public checkout has changes" >&2; exit 1; }
if git -C "$public" rev-parse -q --verify "refs/tags/$tag" >/dev/null; then
    echo "the tag $tag exists in the public checkout" >&2; exit 1
fi

# The new tree replaces the old one whole, so that a file removed here goes there too.
git -C "$public" rm -r -q --ignore-unmatch . >/dev/null
git -C "$root" archive "$commit" -- . ':(exclude)comparison/results' ':(exclude)CLAUDE.md' \
    | tar -x -C "$public"
git -C "$public" add -A
git -C "$public" commit -q -m "Source of the renderer for Fractazm $tag" \
    -m "Tree of commit $commit of the working repository, without comparison/results."
git -C "$public" tag "$tag"
echo "committed and tagged $tag in $public; push with: git -C $public push origin HEAD $tag"
