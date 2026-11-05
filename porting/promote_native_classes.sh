#!/usr/bin/env bash
# Regenerates the promoted native classes from the reflection data.
#
# Inputs (git-ignored, under porting/generated/reflection/): fortnite_game_reflection.json,
# class_name_map.json, class_name_map.evidence.json, class_parent_map.json. Written outputs:
#   porting/native/Source/<Module>/Generated/   one per package: /Script/FortniteGame (FortniteGame),
#       /Script/PlayspaceFramework, and the runtime modules of gen_native_classes.py RUNTIME_MODULE_PACKAGES
#   porting/native/Source/<Module>/<Module>.Build.cs and Private/<Module>.cpp for the runtime modules
#   porting/generated/reflection/class_ref_map.json, skipped_properties.json, native_classes_index.json,
#       required_modules.json
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
porting_dir=$script_dir
reflection_dir="$porting_dir/generated/reflection"
engine_classes="${FN_ENGINE_CLASSES:-/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes}"

# Generate in isolation so failed resolution or generation cannot alter live output.
staging_dir=$(mktemp -d "${TMPDIR:-/tmp}/fortnite-promotion.XXXXXXXX")
trap 'rm -rf -- "$staging_dir"' EXIT
mkdir -p "$staging_dir/Source" "$staging_dir/reports"

python3 -I "$porting_dir/resolve_class_refs.py" \
    --reflection "$reflection_dir/fortnite_game_reflection.json" \
    --evidence "$reflection_dir/class_name_map.evidence.json" \
    --out "$staging_dir/reports/class_ref_map.json"

python3 -I "$porting_dir/gen_native_classes.py" --promote \
    --reflection "$reflection_dir/fortnite_game_reflection.json" \
    --name-map "$reflection_dir/class_name_map.json" \
    --evidence "$reflection_dir/class_name_map.evidence.json" \
    --parent-map "$reflection_dir/class_parent_map.json" \
    --class-ref-map "$staging_dir/reports/class_ref_map.json" \
    --class-list "$porting_dir/native_class_set.txt" \
    --engine-classes "$engine_classes" \
    --source-root "$staging_dir/Source" \
    --report-dir "$staging_dir/reports"

python3 -I "$porting_dir/merge_promoted_classes.py" \
    "$staging_dir/Source" "$porting_dir/native/Source" \
    "$staging_dir/reports" "$reflection_dir"
