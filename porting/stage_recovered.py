#!/usr/bin/env python3
"""Stage selected recovered packages without mixing Windows and Linux payloads."""

from __future__ import annotations

import argparse
import shutil
from pathlib import Path


EXTENSIONS = {".uasset", ".uexp", ".ubulk", ".umap", ".uptnl"}
PREFIX = "FortniteGame/Content/"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package_list", type=Path, help="newline list of package paths without extensions")
    args = parser.parse_args()

    porting = Path(__file__).resolve().parent
    source_root = porting / "generated/extracted/FortniteGame/Content"
    stage_root = porting / "generated/linux-stage/LinuxNoEditor/FortniteLinux/Content"
    staged = skipped = missing = 0

    for raw in args.package_list.read_text().splitlines():
        package = raw.strip()
        if not package or package.startswith("#"):
            continue
        if not package.startswith(PREFIX):
            raise ValueError(f"Not a Fortnite content path: {package}")
        relative = Path(package[len(PREFIX) :])
        if relative.is_absolute() or any(part in {".", ".."} for part in relative.parts) or any(
            char in package for char in "*?[]\\"
        ):
            raise ValueError(f"Unsafe package path: {package}")

        source = source_root / relative
        files = sorted(path for path in source.parent.glob(source.name + ".*") if path.suffix in EXTENSIONS)
        if not files or not any(path.suffix in {".uasset", ".umap"} for path in files):
            print(f"Missing extracted package: {package}")
            missing += 1
            continue

        target_dir = stage_root / relative.parent
        existing = [target_dir / path.name for path in files if (target_dir / path.name).exists()]
        if existing:
            print(f"Already staged, left intact: {package}")
            skipped += 1
            continue

        target_dir.mkdir(parents=True, exist_ok=True)
        for path in files:
            shutil.copy2(path, target_dir / path.name)
        print(f"Staged {package} ({len(files)} files)")
        staged += 1

    print(f"Packages: {staged} staged, {skipped} existing, {missing} missing")
    return 1 if missing else 0


if __name__ == "__main__":
    raise SystemExit(main())
