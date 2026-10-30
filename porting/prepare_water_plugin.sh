#!/usr/bin/env bash
# Rebuild the Water plugin backport (UE 4.26 Experimental/Water on UE 4.25) as a project plugin.
# Epic's plugin source is copied from the local 4.26 checkout and stays out of Git; only our
# changes are tracked: patches/ue425-water-plugin-{backport.patch,removed-files.txt,overlay/}. Usage: prepare_water_plugin.sh [ue426_dir] [dest]
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ue426=${1:-"$script_dir/../../UnrealEngine4260"}
dest=${2:-"$script_dir/native/Plugins/Water"}
src="$ue426/Engine/Plugins/Experimental/Water"
patch_file="$script_dir/patches/ue425-water-plugin-backport.patch"
removed_list="$script_dir/patches/ue425-water-plugin-removed-files.txt"
overlay_dir="$script_dir/patches/ue425-water-plugin-overlay"

if [[ ! -f "$src/Water.uplugin" ]]; then
    printf 'UE 4.26 Water plugin not found: %s\n' "$src" >&2
    exit 2
fi
if [[ -e "$dest" ]]; then
    printf 'Destination exists, remove it first: %s\n' "$dest" >&2
    exit 3
fi
mkdir -p "$dest"
cp -a "$src/." "$dest/"
# Files removed from the port (editor module, Niagara data interface, etc.), then edits, then our own files.
while IFS= read -r rel; do rm -f "$dest/$rel"; done < "$removed_list"
find "$dest" -depth -type d -empty -delete
mkdir -p "$dest"
patch -p1 -d "$dest" < "$patch_file" >/dev/null
cp -a "$overlay_dir/." "$dest/"
printf 'Water plugin backport prepared in %s\n' "$dest"
