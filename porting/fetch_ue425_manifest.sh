#!/usr/bin/env bash
# Fetch the updated UE 4.25 dependency manifest using the caller's Git access.
# Nothing is added to this port repository or either existing engine checkout.
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
default_output=$(cd -- "$script_dir/../.." && pwd)/ue425-Commit.gitdeps.xml
output_path=${1:-$default_output}

for program in git python3 mktemp; do
    if ! command -v "$program" >/dev/null 2>&1; then
        printf 'Missing required program: %s\n' "$program" >&2
        exit 1
    fi
done

if [[ -e "$output_path" ]]; then
    printf 'Output already exists: %s\n' "$output_path" >&2
    exit 1
fi

scratch_dir=$(mktemp -d)
trap 'rm -rf -- "$scratch_dir"' EXIT

printf 'Fetching the latest UE 4.25 branch manifest with your configured Git credentials...\n'
git clone --quiet --filter=blob:none --no-checkout --depth=1 --single-branch \
    --branch 4.25 https://github.com/EpicGames/UnrealEngine.git \
    "$scratch_dir/UnrealEngine"
git -C "$scratch_dir/UnrealEngine" show HEAD:Engine/Build/Commit.gitdeps.xml \
    > "$scratch_dir/Commit.gitdeps.xml"

python3 - "$scratch_dir/Commit.gitdeps.xml" <<'PY'
import sys
import xml.etree.ElementTree as ET

root = ET.parse(sys.argv[1]).getroot()
if root.tag != "DependencyManifest":
    raise SystemExit("Downloaded file is not a UE dependency manifest")
if not root.attrib.get("BaseUrl", "").startswith(("http://", "https://")):
    raise SystemExit("Dependency manifest has no valid BaseUrl")
files = root.find("Files")
if files is None or len(files) < 1000:
    raise SystemExit("Dependency manifest appears incomplete")
print(f"Validated manifest with {len(files)} file records.")
PY

mkdir -p -- "$(dirname -- "$output_path")"
cp -- "$scratch_dir/Commit.gitdeps.xml" "$output_path"
printf 'Saved: %s\n' "$output_path"
printf 'The engine checkout and this port repository were not modified.\n'
