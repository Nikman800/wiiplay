#!/bin/bash
# release_check.sh [DIR]
# Audit a public-release snapshot. With no argument, checks the files git tracks in this repo,
# minus the paths a release leaves out (tools/workers/). With DIR, checks every file under DIR
# (e.g. an exported snapshot). Exits 1 and lists the hits when anything is found.
#
# Checks:
#   1. private terms: extended regexes, one per line, read from a file that is NOT part of the
#      repository ($RELEASE_TERMS, default ~/.config/wiiplay/release-terms; lines starting with
#      "#" are comments, a line starting with "cs:" is matched case-sensitively)
#   2. personal paths (home directories and Windows user folders in any spelling)
#   3. files that must never ship: disc-derived data, split asm, binaries, local agent config
set -u
cd "$(dirname "$0")/.."

EXCLUDE='^(tools/workers/|tools/release_check\.sh$)'
if [ $# -ge 1 ]; then
    ROOT="$1"
    mapfile -t FILES < <(cd "$ROOT" && find . -type f -not -path './.git/*' | sed 's#^\./##' | grep -Ev "$EXCLUDE")
else
    ROOT=.
    mapfile -t FILES < <(git ls-files | grep -Ev "$EXCLUDE")
fi

fail=0
report() { echo "== $1"; echo "$2" | head -50; echo; fail=1; }
search() {  # search <grep flags> <regex>
    (cd "$ROOT" && printf '%s\0' "${FILES[@]}" | xargs -0 grep -I"$1"E "$2" 2>/dev/null)
}

# 1. private terms
TERMS="${RELEASE_TERMS:-$HOME/.config/wiiplay/release-terms}"
if [ -r "$TERMS" ]; then
    while IFS= read -r t; do
        case "$t" in ''|'#'*) continue ;; esac
        if [ "${t#cs:}" != "$t" ]; then hits=$(search n "${t#cs:}"); else hits=$(search in "$t"); fi
        [ -n "$hits" ] && report "private term" "$hits"
    done < "$TERMS"
else
    echo "release_check: WARNING: no private terms file at $TERMS (set RELEASE_TERMS); skipping check 1" >&2
    fail=1
fi

# 2. personal paths: /home/<user>, /Users/<user>, C:\Users\, C:/Users/, /c/Users/, /mnt/c/Users/
PERSONAL='/home/[a-z]|/Users/[A-Za-z]|[A-Za-z]:[\\/]+Users[\\/]|(^|[^A-Za-z0-9_])/[a-z]/Users/|/mnt/[a-z]/Users'
hits=$(search in "$PERSONAL")
[ -n "$hits" ] && report "personal paths" "$hits"

# 3. files that must not ship
FORBIDDEN='^(extracted/|decomp/|decomp_typed/|dtk/split/|build/|orig/|\.claude/|CLAUDE\.md$)|\.(dol|elf|rvz|iso|wbfs|rel|o|bin|arc|brres|tpl)$'
hits=$(printf '%s\n' "${FILES[@]}" | grep -E "$FORBIDDEN")
[ -n "$hits" ] && report "files that must not ship" "$hits"

if [ $fail -eq 0 ]; then
    echo "release_check: clean (${#FILES[@]} files checked)"
fi
exit $fail
