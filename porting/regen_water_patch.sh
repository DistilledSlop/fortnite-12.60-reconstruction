#!/usr/bin/env bash
# Regenerate the tracked Water backport files from the working plugin copy in native/Plugins/Water:
#   patches/ue425-water-plugin-backport.patch        edits to Epic files (diff against the 4.26 original)
#   patches/ue425-water-plugin-removed-files.txt      Epic files deleted from the port
#   patches/ue425-water-plugin-overlay/               files we wrote (replacements and shims)
# Epic's source itself stays out of Git. Usage: regen_water_patch.sh [ue426_dir]
set -euo pipefail
export LC_ALL=C

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ue426=${1:-"$script_dir/../../UnrealEngine4260"}
src="$ue426/Engine/Plugins/Experimental/Water"
work="$script_dir/native/Plugins/Water"
pdir="$script_dir/patches"
# Files we replaced wholesale or added (kept as overlay, excluded from the patch).
overlay_files=(Source/Runtime/Private/WaterBooleanUtils.h Source/Runtime/Private/SimpleSuspensionShim.h)

[[ -f "$src/Water.uplugin" && -f "$work/Water.uplugin" ]] || { echo "missing source or working copy" >&2; exit 2; }
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
mkdir -p "$tmp/a" "$tmp/b"
cp -a "$src/." "$tmp/a/"; cp -a "$work/." "$tmp/b/"
# Build products from editor/game builds are not part of the port.
rm -rf "$tmp/b/Binaries" "$tmp/b/Intermediate"
(cd "$tmp/a" && find . -type f | sort) > "$tmp/a.lst"
(cd "$tmp/b" && find . -type f | sort) > "$tmp/b.lst"
comm -23 "$tmp/a.lst" "$tmp/b.lst" | sed 's#^\./##' > "$tmp/removed.lst"
for f in "${overlay_files[@]}"; do
    [[ -f "$work/$f" ]] && { rel=$f; echo "$rel" >> "$tmp/removed.lst"; }
done
sort -u -o "$tmp/removed.lst" "$tmp/removed.lst"
# Patch base: original minus removed files; patch target: working copy minus overlay files.
(cd "$tmp/a" && xargs -a "$tmp/removed.lst" rm -f)
for f in "${overlay_files[@]}"; do rm -f "$tmp/b/$f"; done
(cd "$tmp" && diff -ruN a b > mod.diff || true)
cp "$tmp/removed.lst" "$pdir/ue425-water-plugin-removed-files.txt"
cp "$tmp/mod.diff" "$pdir/ue425-water-plugin-backport.patch"
rm -rf "$pdir/ue425-water-plugin-overlay"
for f in "${overlay_files[@]}"; do
    [[ -f "$work/$f" ]] && install -D "$work/$f" "$pdir/ue425-water-plugin-overlay/$f"
done
echo "patch: $(wc -l < "$pdir/ue425-water-plugin-backport.patch") lines; removed: $(wc -l < "$pdir/ue425-water-plugin-removed-files.txt") files"
