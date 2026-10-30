#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
cue4parse_dir=${1:-"$script_dir/../../FModel/CUE4Parse"}
patch_file="$script_dir/patches/cue4parse-fortnite-12.60-static-mesh.patch"

if [[ ! -f "$cue4parse_dir/CUE4Parse/CUE4Parse.csproj" ]]; then
    printf 'CUE4Parse checkout not found: %s\n' "$cue4parse_dir" >&2
    exit 2
fi

if git -C "$cue4parse_dir" apply --reverse --check "$patch_file" 2>/dev/null; then
    printf 'Fortnite 12.60 mesh compatibility patch is already applied.\n'
elif git -C "$cue4parse_dir" apply --check "$patch_file"; then
    git -C "$cue4parse_dir" apply "$patch_file"
    printf 'Applied Fortnite 12.60 mesh compatibility patch to local CUE4Parse checkout.\n'
else
    printf 'CUE4Parse checkout does not match the compatibility patch.\n' >&2
    exit 1
fi
