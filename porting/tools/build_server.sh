#!/usr/bin/env bash
# Build the Linux dedicated server and, with --stage, copy it into the server stage used by play runs.
# Log: porting/generated/builds/<YYYYMMDD-HHMMSS>-server.log. Exit status is non-zero on a failed build
# (a failed build leaves the old binary in place, so never stage without checking).
# Usage: tools/build_server.sh [--stage]
set -uo pipefail
porting=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
engine=${UE_ROOT:-/opt/fortnite/UnrealEngine}
log_dir="$porting/generated/builds"
mkdir -p "$log_dir"
log="$log_dir/$(date +%Y%m%d-%H%M%S)-server.log"
echo "Build log: $log"
"$engine/Engine/Build/BatchFiles/Linux/Build.sh" FortniteLinuxServer Linux Development \
    -project="$porting/native/FortniteLinux.uproject" -NoHotReload > "$log" 2>&1
status=$?
diagnostics=$(python3 - "$log" <<'PYDIAGNOSTICS'
import re, sys
ansi = re.compile(r'\x1b\[[0-?]*[ -/]*[@-~]')
errors = []
with open(sys.argv[1], errors='replace') as stream:
    for line in stream:
        clean = ansi.sub('', line)
        if re.search(r'(^|\s)error:', clean):
            errors.append(clean.rstrip())
print(len(errors))
print('\n'.join(errors[:10]))
PYDIAGNOSTICS
)
errors=${diagnostics%%$'\n'*}
if ((status != 0 || errors > 0)); then
    echo "Build FAILED (exit $status, $errors error lines):"
    [[ $errors == 0 ]] || printf '%s\n' "${diagnostics#*$'\n'}"
    exit 1
fi
echo "Build OK ($(tail -1 "$log"))"
if [[ ${1:-} == --stage ]]; then
    python3 -I "$porting/tools/stage_original_plugin_content.py" \
        --extracted "${FN_EXTRACTED_ROOT:-/mnt/vibrato/fortnite-extracted-full}" \
        --stage "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux" || exit 1
    python3 "$porting/tools/stage_gameplay_tags.py" \
        --src-game "${FN_COOKED_GAME_ROOT:-/mnt/vibrato/fortnite-extracted-full/FortniteGame}" \
        --dst-stage "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux" || exit 1
    if [[ ${FN_STAGE_APOLLO_STREAMING:-1} == 1 ]]; then
        # Terrain's hard-import closure omits dynamically loaded maps, including
        # POIs and building instances. Stage every Apollo map as a root so that
        # the original client's visibility updates can resolve every package.
        python3 -I - "$porting" "${FN_COOKED_GAME_ROOT:-/mnt/vibrato/fortnite-extracted-full/FortniteGame}" <<'PYEOF' || exit 1
from pathlib import Path
import subprocess, sys
porting, game = map(Path, sys.argv[1:])
content = game / 'Content'
maps = sorted((content / 'Athena/Apollo').rglob('*.umap'))
if not maps:
    raise SystemExit('Apollo maps are missing; refusing stage')
packages = ['/Game/' + path.relative_to(content).with_suffix('').as_posix() for path in maps]
subprocess.run([sys.executable, '-I', str(porting / 'tools/stage_server_content.py'),
                '--src', str(content), '--max', '30000', '--strict', *packages], check=True)
PYEOF
    fi
    if [[ ${FN_STAGE_BUILDING_CACHE:-1} == 1 ]]; then
        registry_sha1=7dfa4a945e3fcf26cc6f0fd5d7b22f4700bd28b1
        registry_sha256=8cdd9e893b9e043d45702fb285d312476105b448330493cbb872ba3d58fe47b0
        if [[ -n ${FN_BUILDING_CLASS_MANIFEST:-} ]]; then
            registry="$FN_BUILDING_CLASS_MANIFEST"
        else
            extracted_root="${FN_EXTRACTED_ROOT:-/mnt/vibrato/fortnite-extracted-full}"
            registry_bin="${FN_ASSET_REGISTRY_BIN:-$extracted_root/FortniteGame/AssetRegistry.bin}"
            content_dir="${FN_EXTRACTED_CONTENT_DIR:-$extracted_root/FortniteGame/Content}"
            cache_dir="$porting/generated/building-registry"
            tagged_json="${FN_BUILDING_TAGGED_JSON:-$cache_dir/tagged.json}"
            registry="$cache_dir/default-scan-candidates.json"
            mkdir -p "$cache_dir"
            if [[ ! -f $tagged_json ]]; then
                c4i_dir="$cache_dir/cue4inspect"
                dotnet build "$porting/cue4inspect/Cue4Inspect.csproj" -c Release -o "$c4i_dir" > "$cache_dir/cue4inspect-build.log" 2>&1 || exit 1
                dotnet "$c4i_dir/Cue4Inspect.dll" --export-buildable-tags "$registry_bin" --tags-out="$tagged_json" || exit 1
            fi
            python3 -I "$porting/tools/generate_building_class_manifest.py" \
                --tagged "$tagged_json" \
                --registry "$registry_bin" \
                --content-dir "$content_dir" \
                --output "$registry" \
                --expected-registry-sha256 "$registry_sha256" \
                --expected-count 331 \
                --expected-output-sha1 "$registry_sha1" \
                --require-packages-present || exit 1
        fi
        if [[ ! -f $registry ]] || [[ $(sha1sum "$registry" | cut -d ' ' -f 1) != "$registry_sha1" ]]; then
            echo 'Original PlayerBuildingClass registry manifest missing or changed; refusing stage' >&2
            exit 1
        fi
        cp "$registry" "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Config/PlayerBuildingClassRegistry.json" || exit 1
    # The registry is also used to resolve requested construction handles. Stage
    # every candidate as a root; terrain imports do not cover selectable pieces.
    python3 -I - "$porting" "$registry" "${FN_COOKED_GAME_ROOT:-/mnt/vibrato/fortnite-extracted-full/FortniteGame}" <<'PYBUILDINGS' || exit 1
from pathlib import Path
import json, subprocess, sys
porting, registry, game = map(Path, sys.argv[1:])
rows = json.loads(registry.read_text())['rows']
packages = sorted({row['classPathCandidate'].split('.', 1)[0] for row in rows})
if len(rows) != 331 or len(packages) != 331 or any(not p.startswith('/Game/Building/ActorBlueprints/Player/') for p in packages):
    raise SystemExit('Unexpected building registry roots; refusing stage')
subprocess.run([sys.executable, '-I', str(porting / 'tools/stage_server_content.py'),
                '--src', str(game / 'Content'), '--max', '30000', '--strict', *packages], check=True)
PYBUILDINGS
    fi
    if [[ ${FN_STAGE_CHARACTER_CACHE:-0} == 1 ]]; then
        char_registry_sha1=aafb2f7788864605068d85fccd1de7e627f18682
        char_registry_sha256=8cdd9e893b9e043d45702fb285d312476105b448330493cbb872ba3d58fe47b0
        if [[ -n ${FN_CHARACTER_MANIFEST:-} ]]; then
            char_registry="$FN_CHARACTER_MANIFEST"
        else
            extracted_root="${FN_EXTRACTED_ROOT:-/mnt/vibrato/fortnite-extracted-full}"
            registry_bin="${FN_ASSET_REGISTRY_BIN:-$extracted_root/FortniteGame/AssetRegistry.bin}"
            content_dir="${FN_EXTRACTED_CONTENT_DIR:-$extracted_root/FortniteGame/Content}"
            char_cache_dir="$porting/generated/character-registry"
            char_tagged_json="${FN_CHARACTER_TAGGED_JSON:-$char_cache_dir/tagged.json}"
            char_registry="$char_cache_dir/AthenaCharacterRegistry.json"
            mkdir -p "$char_cache_dir"
            if [[ ! -f $char_tagged_json ]]; then
                char_c4i_dir="$char_cache_dir/cue4inspect"
                dotnet build "$porting/cue4inspect/Cue4Inspect.csproj" -c Release -o "$char_c4i_dir" > "$char_cache_dir/cue4inspect-build.log" 2>&1 || exit 1
                dotnet "$char_c4i_dir/Cue4Inspect.dll" --export-character-tags "$registry_bin" --tags-out="$char_tagged_json" || exit 1
            fi
            python3 -I "$porting/tools/generate_character_manifest.py" \
                --tagged "$char_tagged_json" \
                --registry "$registry_bin" \
                --content-dir "$content_dir" \
                --output "$char_registry" \
                --expected-registry-sha256 "$char_registry_sha256" \
                --expected-count 952 \
                --expected-output-sha1 "$char_registry_sha1" \
                --require-packages-present || exit 1
        fi
        if [[ ! -f $char_registry ]] || [[ $(sha1sum "$char_registry" | cut -d ' ' -f 1) != "$char_registry_sha1" ]]; then
            echo 'Original AthenaCharacter registry manifest missing or changed; refusing stage' >&2
            exit 1
        fi
        cp "$char_registry" "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Config/AthenaCharacterRegistry.json" || exit 1
    fi
    cp "$porting/native/Binaries/Linux/FortniteLinuxServer" \
       "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/FortniteLinuxServer"
    cp "$porting/native/Config/OriginalDefaults.json" \
       "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Config/OriginalDefaults.json"
    mkdir -p "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Config/Linux"
    cp "$porting/native/Config/Linux/LinuxEngine.ini" "$porting/native/Config/Linux/LinuxGame.ini" \
       "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Config/Linux/"
    echo "Staged into generated/linux-stage."
fi
