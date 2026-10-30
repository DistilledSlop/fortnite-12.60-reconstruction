#!/usr/bin/env python3
"""Inventory the supplied Fortnite 12.60 build without extracting assets.

The v10 pak footer layout is documented by UE's FPakInfo and the later
PathHashIndex format. This only reads the trailing metadata; it never scans
or copies package contents and never handles encryption keys.
"""

from __future__ import annotations

import argparse
import json
import struct
import subprocess
from pathlib import Path


PAK_MAGIC = 0x5A6F12E1
PAK_V10_FOOTER_SIZE = 221
COMPRESSION_NAME_SIZE = 32
COMPRESSION_METHODS = 5


def inspect_pak(path: Path) -> dict:
    size = path.stat().st_size
    if size < PAK_V10_FOOTER_SIZE:
        raise ValueError(f"{path.name}: too small for a v10 pak footer")

    with path.open("rb") as stream:
        stream.seek(-PAK_V10_FOOTER_SIZE, 2)
        footer = stream.read(PAK_V10_FOOTER_SIZE)

    key_guid = footer[:16].hex()
    encrypted_index = bool(footer[16])
    magic, version, index_offset, index_size = struct.unpack_from("<IIQQ", footer, 17)
    if magic != PAK_MAGIC or version != 10:
        raise ValueError(
            f"{path.name}: expected a v10 footer, found magic={magic:#x}, version={version}"
        )
    if index_offset + index_size > size - PAK_V10_FOOTER_SIZE:
        raise ValueError(f"{path.name}: index extends past the pak footer")

    method_bytes = footer[61:]
    methods = [
        method_bytes[i : i + COMPRESSION_NAME_SIZE].split(b"\0", 1)[0]
        .decode("ascii", "replace")
        for i in range(0, COMPRESSION_NAME_SIZE * COMPRESSION_METHODS, COMPRESSION_NAME_SIZE)
    ]
    return {
        "name": path.name,
        "size_bytes": size,
        "pak_version": version,
        "index_encrypted": encrypted_index,
        "key_guid": key_guid,
        "index_offset": index_offset,
        "index_size": index_size,
        "compression_methods": [name for name in methods if name],
    }


def engine_info(path: Path) -> dict:
    build_version = path / "Engine/Build/Build.version"
    if not build_version.exists():
        return {"present": False}
    result = {"present": True, "build_version": json.loads(build_version.read_text())}
    revision = subprocess.run(
        ["git", "-C", str(path), "rev-parse", "HEAD"],
        capture_output=True,
        text=True,
        check=False,
    )
    if revision.returncode == 0:
        result["git_revision"] = revision.stdout.strip()
    return result


def inspect(game_root: Path, engine_root: Path) -> dict:
    pak_dir = game_root / "Fortnite/FortniteGame/Content/Paks"
    binary_dir = game_root / "Fortnite/FortniteGame/Binaries/Win64"
    paks = [inspect_pak(path) for path in sorted(pak_dir.glob("*.pak"))]
    return {
        "build_directory": game_root.name,
        "engine": engine_info(engine_root),
        "pak_count": len(paks),
        "signature_count": len(list(pak_dir.glob("*.sig"))),
        "all_indexes_encrypted": all(pak["index_encrypted"] for pak in paks),
        "pak_versions": sorted({pak["pak_version"] for pak in paks}),
        "windows_executables": sorted(path.name for path in binary_dir.glob("*.exe")),
        "paks": paks,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    default_game_root = Path(__file__).resolve().parent.parent
    parser.add_argument("--game-root", type=Path, default=default_game_root)
    parser.add_argument("--engine-root", type=Path, default=default_game_root.parent / "UnrealEngine")
    args = parser.parse_args()
    print(json.dumps(inspect(args.game_root, args.engine_root), indent=2))


if __name__ == "__main__":
    main()
