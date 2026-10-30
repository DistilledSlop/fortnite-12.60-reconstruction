#!/usr/bin/env python3
"""Create UE ImportAssets settings for recovered Apollo landscape GLBs."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


DESTINATION = "/Game/Athena/Apollo/Maps/Landscape/Converted"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--limit", type=int, help="select the first N unimported meshes")
    args = parser.parse_args()
    if args.limit is not None and args.limit < 1:
        parser.error("--limit must be positive")

    porting = Path(__file__).resolve().parent
    converted = porting / "generated/converted"
    manifest = json.loads((converted / "apollo-landscape-grid.json").read_text())
    content = porting / "native/Content/Athena/Apollo/Maps/Landscape/Converted"
    selected: list[str] = []
    for entry in manifest:
        tile = entry["tile"]
        actor = entry["actor"]
        if not re.fullmatch(r"[A-H][1-8]", tile) or actor != f"LandscapeStreamingProxy_{tile}":
            raise ValueError(f"Unexpected tile metadata: {tile}, {actor}")
        source = converted / "FortniteGame/Content/Athena/Apollo/Maps/Landscape" / f"Apollo_Terrain_LS_{tile}" / f"{actor}.glb"
        if not source.is_file() or source.stat().st_size < 1000:
            raise ValueError(f"Missing or invalid GLB: {source}")
        with source.open("rb") as stream:
            if stream.read(4) != b"glTF":
                raise ValueError(f"Invalid GLB header: {source}")
        if (content / f"{actor}.uasset").exists():
            continue
        selected.append(str(source.resolve()))
        if args.limit is not None and len(selected) >= args.limit:
            break

    target = porting / "generated/import-apollo-grid.json"
    if not selected:
        target.write_text(json.dumps({"ImportGroups": []}, indent=2) + "\n")
        print("All available Apollo grid meshes already have editor source assets")
        return 0
    settings = {
        "ImportGroups": [{
            "GroupName": "ApolloLandscapeGrid",
            "Filenames": selected,
            "DestinationPath": DESTINATION,
            "FactoryName": "GLTFImportFactory",
            "bReplaceExisting": False,
        }]
    }
    target.write_text(json.dumps(settings, indent=2) + "\n")
    print(f"Prepared {len(selected)} GLB imports in {target}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
