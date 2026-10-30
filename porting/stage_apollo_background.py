#!/usr/bin/env python3
"""Stage native cooked Apollo background meshes and their placement manifest."""

from __future__ import annotations

import json
import shutil
from pathlib import Path


PAYLOAD_SUFFIXES = {".uasset", ".uexp", ".ubulk", ".uptnl"}


def main() -> int:
    porting = Path(__file__).resolve().parent
    manifest = porting / "generated/converted/apollo-background-placements.json"
    entries = json.loads(manifest.read_text())
    mesh_names = sorted({entry["mesh"] for entry in entries})
    cooked = porting / "native/Saved/Cooked/LinuxNoEditor/FortniteLinux/Content"
    staged_game = porting / "generated/linux-stage/LinuxNoEditor/FortniteLinux"
    for mesh in mesh_names:
        package = cooked / "Athena/Apollo/Maps/Background/Converted" / mesh
        payload = [path for path in package.parent.glob(package.name + ".*")
                   if path.suffix in PAYLOAD_SUFFIXES]
        if not any(path.suffix == ".uasset" for path in payload):
            raise FileNotFoundError(f"Missing cooked background mesh: {package}")
        for source in payload:
            target = staged_game / "Content" / source.relative_to(cooked)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
    target = staged_game / "Config/ApolloBackground.json"
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(manifest, target)
    print(f"Staged {len(mesh_names)} background mesh packages and {len(entries)} placements")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
