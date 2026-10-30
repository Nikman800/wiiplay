#!/bin/bash
# Fetch symbol/split tables from CC0 community decompilations used as naming references
# (Phase 2 library identification). Pinned to commits in ref/COMMITS.tsv when present;
# otherwise records the current HEAD of each repo there.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REF="$ROOT/ref"
mkdir -p "$REF"
# repo  config-version
# local-name  owner/repo  config-version
# petari (Super Mario Galaxy, Nov 2007): closest library versions to Wii Play Rev 1 (mid-2007)
SOURCES="ogws doldecomp/ogws RSPE01_01
mkw doldecomp/mkw RMCP01
brawl doldecomp/brawl RSBE01_02
forecast doldecomp/forecast HAFE
petari SMGCommunity/Petari RMGE01"
touch "$REF/COMMITS.tsv"
while read -r repo slug ver; do
  sha=$(awk -v r="$repo" '$1==r {print $2}' "$REF/COMMITS.tsv")
  if [ -z "$sha" ]; then
    sha=$(gh api "repos/$slug/commits/HEAD" --jq .sha)
    printf '%s\t%s\n' "$repo" "$sha" >> "$REF/COMMITS.tsv"
  fi
  mkdir -p "$REF/$repo"
  for f in symbols splits; do
    curl -fsSL "https://raw.githubusercontent.com/$slug/$sha/config/$ver/$f.txt" -o "$REF/$repo/$f.txt"
  done
  echo "$repo @ ${sha:0:10}: $(wc -l < "$REF/$repo/symbols.txt") symbols, $(grep -c '^[^ 	].*:$' "$REF/$repo/splits.txt") units"
done <<< "$SOURCES"

# Phase 4: Wii Sports' RP framework source (CC0) as the starting point for the shared units,
# plus its configure.py (which units match there). Sparse checkout of src/Pack at the pinned commit.
sha=$(awk '$1=="ogws" {print $2}' "$REF/COMMITS.tsv")
if [ ! -d "$REF/ogws-src/src/Pack" ]; then
  mkdir -p "$REF/ogws-src"
  git -C "$REF/ogws-src" init -q
  git -C "$REF/ogws-src" remote add origin https://github.com/doldecomp/ogws.git
  git -C "$REF/ogws-src" sparse-checkout set src/Pack
  git -C "$REF/ogws-src" fetch -q --depth 1 origin "$sha"
  git -C "$REF/ogws-src" checkout -q FETCH_HEAD
fi
curl -fsSL "https://raw.githubusercontent.com/doldecomp/ogws/$sha/configure.py" -o "$REF/ogws-src/configure.py"
echo "ogws source @ ${sha:0:10}: $(find "$REF/ogws-src/src/Pack" -name '*.cpp' | wc -l) RP source files"
