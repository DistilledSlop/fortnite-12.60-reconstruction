#!/usr/bin/env python3
"""Bake a visual proxy for one converted Apollo tile from recovered layer masks."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

from PIL import Image


PIXELS_PER_COMPONENT = 512
REPEATS_PER_COMPONENT = 4
TEXTURE_PATHS = {
    "Sand_LayerInfo": "T_Apollo_Sand_D_01",
    "Gravel_LayerInfo": "T_Apollo_Gravel_D_01",
    "Mud_LayerInfo": "T_Apollo_Dirt_D_01",
    "Asphalt_LayerInfo": "T_AsphaltDetail1_D",
    "BiomeA_LayerInfo": "T_Apollo_Grass_D_01",
    "BiomeB_LayerInfo": "T_Apollo_Forest_Floor_D_01",
    "BiomeC_LayerInfo": "T_Apollo_Grass_D_01",
    "BiomeD_LayerInfo": "T_Apollo_Cliff_D_01",
    "DataLayer": "T_Apollo_Grass_D_01",
}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tile", help="Apollo tile ID, such as D4")
    args = parser.parse_args()
    if not re.fullmatch(r"[A-H][1-8]", args.tile):
        parser.error("tile must be A1 through H8")

    porting = Path(__file__).resolve().parent
    converted = porting / "generated/converted/FortniteGame/Content"
    masks = converted / "Athena/Apollo/Maps/Landscape" / f"Apollo_Terrain_LS_{args.tile}" / "weightmaps"
    components = json.loads((masks / "layers.json").read_text())
    if not components:
        raise ValueError(f"No landscape components for {args.tile}")

    texture_dirs = [
        converted / "Environments/Apollo/Sets/Terrain/Textures",
        converted / "Athena/Apollo/Environments/Landscape/Textures/Roads",
    ]
    sources: dict[str, Image.Image] = {}
    for name in set(TEXTURE_PATHS.values()):
        source = next((directory / f"{name}.png" for directory in texture_dirs
                       if (directory / f"{name}.png").is_file()), None)
        if source is None:
            raise FileNotFoundError(f"Decode the original {name} texture first")
        with Image.open(source) as image:
            patch = image.convert("RGB").resize(
                (PIXELS_PER_COMPONENT // REPEATS_PER_COMPONENT,) * 2,
                Image.Resampling.LANCZOS,
            )
        repeated = Image.new("RGB", (PIXELS_PER_COMPONENT, PIXELS_PER_COMPONENT))
        for y in range(REPEATS_PER_COMPONENT):
            for x in range(REPEATS_PER_COMPONENT):
                repeated.paste(patch, (x * patch.width, y * patch.height))
        sources[name] = repeated

    min_x = min(component["sectionBase"][0] for component in components)
    min_y = min(component["sectionBase"][1] for component in components)
    offsets = []
    for component in components:
        dx = component["sectionBase"][0] - min_x
        dy = component["sectionBase"][1] - min_y
        if dx % 127 or dy % 127:
            raise ValueError(f"Unexpected component spacing in {args.tile}")
        offsets.append((dx // 127, dy // 127))
    width = 1 + max(x for x, _ in offsets)
    height = 1 + max(y for _, y in offsets)
    atlas = Image.new("RGB", (width * PIXELS_PER_COMPONENT, height * PIXELS_PER_COMPONENT))
    for component, (x, y) in zip(components, offsets):
        result = sources["T_Apollo_Grass_D_01"].copy()
        layers = component["layers"]
        for layer in layers:
            source_name = TEXTURE_PATHS.get(layer["name"])
            if source_name is None:
                raise ValueError(f"Unknown landscape layer: {layer['name']}")
            filename = component["textures"][layer["textureIndex"]]
            with Image.open(masks / filename) as image:
                mask = image.convert("RGBA").getchannel(layer["channel"]).resize(
                    (PIXELS_PER_COMPONENT, PIXELS_PER_COMPONENT), Image.Resampling.BILINEAR
                )
            result = Image.composite(sources[source_name], result, mask)
        atlas.paste(result, (x * PIXELS_PER_COMPONENT, y * PIXELS_PER_COMPONENT))

    target = porting / "generated/converted/apollo-baked" / f"Terrain_{args.tile}.png"
    target.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(target, optimize=True)
    print(f"Baked {args.tile}: {len(components)} components, {atlas.width}x{atlas.height}, {target}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
