#!/usr/bin/env python3
"""Stage four locally cooked Apollo Blueprints at their original package paths."""

from __future__ import annotations

import shutil
from pathlib import Path

from probe_apollo_blueprints import BLUEPRINTS


def main() -> int:
    porting = Path(__file__).resolve().parent
    cooked = porting / "native/Saved/Cooked/LinuxNoEditor/FortniteLinux/Content"
    staged = porting / "generated/linux-stage/LinuxNoEditor/FortniteLinux/Content"
    suffixes = {".uasset", ".uexp", ".ubulk", ".uptnl"}
    packages: list[tuple[str, list[Path], list[Path]]] = []

    # Validate every package before replacing any staged file.
    for name, stem in BLUEPRINTS.items():
        source = cooked / stem
        destination = staged / stem
        source_files = sorted(path for path in source.parent.glob(source.name + ".*")
                              if path.suffix in suffixes)
        target_files = sorted(path for path in destination.parent.glob(destination.name + ".*")
                              if path.suffix in suffixes)
        if not any(path.suffix == ".uasset" for path in source_files):
            raise FileNotFoundError(f"Missing Linux cooked Blueprint: {source}")
        if target_files and {path.suffix for path in source_files} != {path.suffix for path in target_files}:
            raise ValueError(f"Payload set differs for {name}; check the staged package before replacing it")
        packages.append((name, source_files, target_files))

    for name, source_files, _ in packages:
        for source in source_files:
            target = staged / source.relative_to(cooked)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
        print(f"Staged Linux Blueprint {name} ({len(source_files)} files)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
