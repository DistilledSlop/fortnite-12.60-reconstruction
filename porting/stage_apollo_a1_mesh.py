#!/usr/bin/env python3
"""Stage the Linux cooked A1 terrain mesh, material, and shader archives."""

from __future__ import annotations

import shutil
from pathlib import Path


PACKAGE_STEMS = (
    "Athena/Apollo/Maps/Landscape/Converted/LandscapeStreamingProxy_A1",
    "Athena/Apollo/Maps/Landscape/Converted/Materials/M_CommonLandscape",
)
PAYLOAD_SUFFIXES = {".uasset", ".uexp", ".ubulk", ".uptnl"}


def main() -> int:
    porting = Path(__file__).resolve().parent
    cooked = porting / "native/Saved/Cooked/LinuxNoEditor/FortniteLinux/Content"
    staged = porting / "generated/linux-stage/LinuxNoEditor/FortniteLinux/Content"
    files: list[Path] = []
    for stem in PACKAGE_STEMS:
        package = cooked / stem
        payload = sorted(path for path in package.parent.glob(package.name + ".*")
                         if path.suffix in PAYLOAD_SUFFIXES)
        if not any(path.suffix == ".uasset" for path in payload):
            raise FileNotFoundError(f"Missing cooked package: {package}")
        files.extend(payload)
    archives = sorted(cooked.glob("ShaderArchive-*-SF_VULKAN_SM5.ushaderbytecode"))
    if len(archives) != 2:
        raise FileNotFoundError("Expected global and game Vulkan shader archives")
    files.extend(archives)

    for source in files:
        target = staged / source.relative_to(cooked)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        print(f"Staged {target.relative_to(staged)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
