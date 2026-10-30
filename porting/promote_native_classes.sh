#!/usr/bin/env bash
# Regenerates the promoted native classes from the reflection data.
#
# Inputs (git-ignored, under porting/generated/reflection/): fortnite_game_reflection.json,
# class_name_map.json, class_name_map.evidence.json, class_parent_map.json. Written outputs:
#   porting/native/Source/FortniteGame/Generated/        (package /Script/FortniteGame)
#   porting/native/Source/PlayspaceFramework/Generated/  (package /Script/PlayspaceFramework)
#   porting/generated/reflection/class_ref_map.json, skipped_properties.json, native_classes_index.json
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
porting_dir=$script_dir
reflection_dir="$porting_dir/generated/reflection"
engine_classes="${FN_ENGINE_CLASSES:-/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes}"

# Only generated trees are removed. Hand-written sources stay.
rm -rf "$porting_dir/native/Source/FortniteGame/Generated" \
       "$porting_dir/native/Source/PlayspaceFramework/Generated"

python3 -I "$porting_dir/resolve_class_refs.py" \
    --reflection "$reflection_dir/fortnite_game_reflection.json" \
    --evidence "$reflection_dir/class_name_map.evidence.json" \
    --out "$reflection_dir/class_ref_map.json"

python3 -I "$porting_dir/gen_native_classes.py" --promote \
    --reflection "$reflection_dir/fortnite_game_reflection.json" \
    --name-map "$reflection_dir/class_name_map.json" \
    --evidence "$reflection_dir/class_name_map.evidence.json" \
    --parent-map "$reflection_dir/class_parent_map.json" \
    --class-ref-map "$reflection_dir/class_ref_map.json" \
    --class-list "$porting_dir/native_class_set.txt" \
    --engine-classes "$engine_classes" \
    --source-root "$porting_dir/native/Source" \
    --report-dir "$reflection_dir"
