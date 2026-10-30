#!/usr/bin/env python3
"""Stage locally cooked Apollo grid meshes and their ignored transform manifest."""

from __future__ import annotations

import json
import re
import shutil
from pathlib import Path


PAYLOAD_SUFFIXES = {".uasset", ".uexp", ".ubulk", ".uptnl"}


def main() -> int:
    porting = Path(__file__).resolve().parent
    manifest_path = porting / "generated/converted/apollo-landscape-grid.json"
    entries = json.loads(manifest_path.read_text())
    if len(entries) != 61:
        raise ValueError(f"Expected 61 nonempty Apollo grid meshes, got {len(entries)}")
    cooked = porting / "native/Saved/Cooked/LinuxNoEditor/FortniteLinux/Content"
    staged_game = porting / "generated/linux-stage/LinuxNoEditor/FortniteLinux"
    staged = staged_game / "Content"
    stems = []
    for entry in entries:
        tile = entry["tile"]
        actor = entry["actor"]
        if not re.fullmatch(r"[A-H][1-8]", tile) or actor != f"LandscapeStreamingProxy_{tile}":
            raise ValueError(f"Invalid tile metadata: {tile}, {actor}")
        stems.append(f"Athena/Apollo/Maps/Landscape/Converted/{actor}")
    material_dir = cooked / "Athena/Apollo/Maps/Landscape/Converted/Materials"
    materials = sorted(material_dir.glob("*.uasset"))
    if not materials:
        raise FileNotFoundError(f"Missing cooked terrain materials: {material_dir}")
    stems.extend(str(material.relative_to(cooked).with_suffix("")) for material in materials)

    files: list[Path] = []
    for stem in stems:
        package = cooked / stem
        payload = sorted(path for path in package.parent.glob(package.name + ".*")
                         if path.suffix in PAYLOAD_SUFFIXES)
        if not any(path.suffix == ".uasset" for path in payload):
            raise FileNotFoundError(f"Missing Linux cooked package: {package}")
        files.extend(payload)
    archives = sorted(cooked.glob("ShaderArchive-*-SF_VULKAN_SM5.ushaderbytecode"))
    if len(archives) != 2:
        raise FileNotFoundError("Expected global and game Vulkan shader archives")
    files.extend(archives)

    for source in files:
        target = staged / source.relative_to(cooked)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    config_manifest = staged_game / "Config/ApolloLandscapeGrid.json"
    config_manifest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(manifest_path, config_manifest)
    print(f"Staged {len(entries)} terrain meshes, {len(materials)} materials, shader archives, and {config_manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
