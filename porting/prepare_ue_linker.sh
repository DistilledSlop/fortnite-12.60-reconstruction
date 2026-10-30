#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
engine_dir=${1:-"$script_dir/../../UnrealEngine"}
patch_file="$script_dir/patches/ue425-linker-diagnostics.patch"

if [[ ! -f "$engine_dir/Engine/Source/Runtime/CoreUObject/Public/UObject/Linker.h" ]]; then
    printf 'UE 4.25 source checkout not found: %s\n' "$engine_dir" >&2
    exit 2
fi

if git -C "$engine_dir" apply --reverse --check "$patch_file" 2>/dev/null; then
    printf 'Linker diagnostic patch is already applied.\n'
elif git -C "$engine_dir" apply --check "$patch_file"; then
    git -C "$engine_dir" apply "$patch_file"
    printf 'Applied linker diagnostic patch to local UE checkout.\n'
else
    printf 'UE checkout does not match the linker diagnostic patch.\n' >&2
    exit 1
fi
