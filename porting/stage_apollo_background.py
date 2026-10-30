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
    background = cooked / "Athena/Apollo/Maps/Background/Converted"
    for mesh in mesh_names:
        if not (background / f"{mesh}.uasset").is_file():
            package = background / mesh
            raise FileNotFoundError(f"Missing cooked background mesh: {package}")
    payload = [path for path in background.rglob("*") if path.suffix in PAYLOAD_SUFFIXES]
    for source in payload:
        target = staged_game / "Content" / source.relative_to(cooked)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    archives = sorted(cooked.glob("ShaderArchive-*-SF_VULKAN_SM5.ushaderbytecode"))
    if len(archives) != 2:
        raise FileNotFoundError("Expected global and game Vulkan shader archives")
    for source in archives:
        shutil.copy2(source, staged_game / "Content" / source.name)
    target = staged_game / "Config/ApolloBackground.json"
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(manifest, target)
    print(f"Staged {len(mesh_names)} background meshes, {len(payload)} cooked payload files, "
          f"{len(entries)} placements, and shader archives")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
