#!/usr/bin/env python3
"""Prepare the rebuilt Apollo cloud and backdrop material spec.

Reads the ignored material reports written by `cue4inspect --material-report`
and writes `generated/apollo-background-materials.json` for the editor
commandlet `FortniteBackgroundMaterials`, plus texture import settings for the
decoded PNGs it needs. Only values that survive in the cooked packages are
copied; the graphs themselves are simplified reconstructions in C++.
"""

from __future__ import annotations

import json
import re
from pathlib import Path


CLOUD_DIR = "/Game/Athena/Environments/Materials/Cloud"
BACKDROP_DIR = "/Game/Athena/Apollo/Environments/Landscape/Background"
CLOUD_TEXTURE = "/Game/VisualThreatManager/StormVisuals/LargeClouds/T_StormCloudMobile"
CLIFF_TEXTURE = "/Game/Environments/Apollo/Sets/Terrain/Textures/T_Apollo_Cliff_D_01"


def srgb_hex_to_linear(value: str) -> list[float]:
    """CUE4Parse prints FLinearColor defaults as 8-bit sRGB hex; undo that."""
    match = re.match(r"([0-9A-Fa-f]{6})\b", value)
    if not match:
        raise ValueError(f"Unexpected color default: {value!r}")
    channels = [int(match.group(1)[i:i + 2], 16) / 255.0 for i in (0, 2, 4)]
    linear = [c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4 for c in channels]
    return [round(c, 6) for c in linear] + [1.0]


def parameter_defaults(report: dict) -> tuple[dict, dict]:
    scalars: dict[str, float] = {}
    vectors: dict[str, list[float]] = {}
    for expression in report["expressionExports"]:
        props = {p["name"]: p["value"] for p in expression["properties"]}
        name = props.get("ParameterName", "").split(" (")[0]
        default = props.get("DefaultValue")
        if not name or default is None:
            continue
        if expression["exportType"] == "MaterialExpressionScalarParameter":
            scalars[name] = float(default.split(" (")[0])
        elif expression["exportType"] == "MaterialExpressionVectorParameter":
            vectors[name] = srgb_hex_to_linear(default)
    return scalars, vectors


def instance(report: dict, package: str, parent: str) -> dict:
    values = report["instanceParameters"][0]
    return {
        "package": package,
        "parent": parent,
        "scalars": {p["parameter"]: p["value"] for p in values["scalars"]},
        "vectors": {p["parameter"]: p["value"] for p in values["vectors"]},
        "textures": {p["parameter"]: p["package"] for p in values["textures"]},
    }


def main() -> int:
    porting = Path(__file__).resolve().parent
    generated = porting / "generated"

    def load(name: str) -> dict:
        return json.loads((generated / f"material-{name}.json").read_text())

    cloud = load("M_Volcano_Cloud")
    scalars, vectors = parameter_defaults(cloud)
    spec = {
        "cloud": {
            "package": f"{CLOUD_DIR}/M_Volcano_Cloud",
            "texture": CLOUD_TEXTURE,
            "scalars": scalars,
            "vectors": vectors,
        },
        "instances": [instance(load("M_Volcano_Cloud_Cove"), f"{CLOUD_DIR}/M_Volcano_Cloud_Cove",
                               f"{CLOUD_DIR}/M_Volcano_Cloud")],
        "backdrop": {
            "package": f"{BACKDROP_DIR}/M_BackDrop_P",
            "texture": CLIFF_TEXTURE,
        },
    }
    for name, parent in (("M_BackDrop_1", "M_BackDrop_P"), ("M_BackDrop_3", "M_BackDrop_P"),
                         ("M_BackDrop_4", "M_BackDrop_3")):
        spec["instances"].append(instance(load(name), f"{BACKDROP_DIR}/{name}", f"{BACKDROP_DIR}/{parent}"))

    (generated / "apollo-background-materials.json").write_text(json.dumps(spec, indent=2) + "\n")

    textures = []
    for package, source in (
        (CLOUD_TEXTURE, "VisualThreatManager/StormVisuals/LargeClouds/T_StormCloudMobile.png"),
        (CLIFF_TEXTURE, "Environments/Apollo/Sets/Terrain/Textures/T_Apollo_Cliff_D_01.png"),
    ):
        png = generated / "converted/FortniteGame/Content" / source
        if png.read_bytes()[:8] != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"Invalid PNG: {png}")
        destination = package.rsplit("/", 1)[0]
        content = porting / "native/Content" / package.removeprefix("/Game/")
        if not content.with_suffix(".uasset").exists():
            textures.append({
                "GroupName": package.rsplit("/", 1)[1],
                "Filenames": [str(png.resolve())],
                "DestinationPath": destination,
                "FactoryName": "TextureFactory",
                "bReplaceExisting": False,
            })
    (generated / "import-apollo-background-textures.json").write_text(
        json.dumps({"ImportGroups": textures}, indent=2) + "\n")
    print(f"Prepared {len(scalars)} scalar and {len(vectors)} vector cloud parameters, "
          f"{len(spec['instances'])} instances, {len(textures)} texture imports")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
