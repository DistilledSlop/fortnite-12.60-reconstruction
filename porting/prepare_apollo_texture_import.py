#!/usr/bin/env python3
"""Prepare a batch UE import for baked Apollo terrain tile atlases."""

from __future__ import annotations

import json
import re
import struct
from pathlib import Path


def main() -> int:
    porting = Path(__file__).resolve().parent
    generated = porting / "generated"
    manifest = json.loads((generated / "converted/apollo-landscape-grid.json").read_text())
    content = porting / "native/Content/Athena/Apollo/Maps/Landscape/Converted/Textures"
    selected: list[str] = []
    for entry in manifest:
        tile = entry["tile"]
        if not re.fullmatch(r"[A-H][1-8]", tile):
            raise ValueError(f"Invalid tile ID: {tile}")
        atlas = generated / "converted/apollo-baked" / f"Terrain_{tile}.png"
        with atlas.open("rb") as stream:
            header = stream.read(24)
        if header[:8] != b"\x89PNG\r\n\x1a\n" or len(header) != 24:
            raise ValueError(f"Invalid atlas PNG: {atlas}")
        width, height = struct.unpack(">II", header[16:24])
        if width < 512 or height < 512 or width > 4096 or height > 4096:
            raise ValueError(f"Unexpected atlas size {width}x{height}: {atlas}")
        if not (content / f"Terrain_{tile}.uasset").exists():
            selected.append(str(atlas.resolve()))

    target = generated / "import-apollo-texture-grid.json"
    settings = {
        "ImportGroups": [{
            "GroupName": "ApolloTerrainAtlases",
            "Filenames": selected,
            "DestinationPath": "/Game/Athena/Apollo/Maps/Landscape/Converted/Textures",
            "FactoryName": "TextureFactory",
            "bReplaceExisting": False,
        }] if selected else []
    }
    target.write_text(json.dumps(settings, indent=2) + "\n")
    print(f"Prepared {len(selected)} texture imports in {target}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
