#!/usr/bin/env python3
"""Prepare native imports and placement data from an Apollo_Background actor report.

The report and all exported GLBs are supplied game data under the ignored
generated tree. This script contains only the conversion logic.
"""

from __future__ import annotations

import json
import re
from pathlib import Path


DESTINATION = "/Game/Athena/Apollo/Maps/Background/Converted"
MESH_NAME = re.compile(r"[A-Za-z][A-Za-z0-9_]*\Z")


def main() -> int:
    porting = Path(__file__).resolve().parent
    generated = porting / "generated"
    report_text = (generated / "apollo-background-actors.json").read_text()
    # The inspector prints its pak mount summary before the JSON report.
    report = json.loads(report_text[report_text.index("{"):])
    components = report["staticMeshComponents"]
    converted = generated / "converted"
    editor_content = porting / "native/Content/Athena/Apollo/Maps/Background/Converted"
    mesh_sources: dict[str, Path] = {}
    placements = []
    skipped = []

    for component in components:
        package = component.get("meshPackage")
        mesh = component.get("mesh")
        if not isinstance(package, str) or not package.startswith("/Game/"):
            skipped.append(component["name"])
            continue
        if not isinstance(mesh, str) or not MESH_NAME.fullmatch(mesh):
            raise ValueError(f"Invalid mesh name in actor report: {mesh!r}")
        package_name = package.rsplit("/", 1)[-1]
        if mesh != package_name:
            # An imported object in another package needs its own export pass.
            skipped.append(component["name"])
            continue
        source = converted / "FortniteGame/Content" / package.removeprefix("/Game/")
        source = source.with_suffix(".glb")
        with source.open("rb") as stream:
            if stream.read(4) != b"glTF":
                raise ValueError(f"Invalid GLB: {source}")
        old = mesh_sources.setdefault(mesh, source)
        if old != source:
            raise ValueError(f"Mesh name collides across packages: {mesh}")
        overrides = component.get("overrideMaterials", [])
        first_override = overrides[0] if overrides else None
        placements.append({
            "mesh": mesh,
            "owner": component["owner"],
            "component": component["name"],
            "materialPackage": first_override.get("package") if first_override else None,
            "materialName": first_override.get("name") if first_override else None,
            "location": component["location"],
            "rotation": component["rotation"],
            "scale": component["scale"],
        })

    manifest = converted / "apollo-background-placements.json"
    manifest.write_text(json.dumps(placements, indent=2) + "\n")
    sources = [str(source.resolve()) for mesh, source in sorted(mesh_sources.items())
               if not (editor_content / f"{mesh}.uasset").exists()]
    settings = {"ImportGroups": [{
        "GroupName": "ApolloBackground",
        "Filenames": sources,
        "DestinationPath": DESTINATION,
        "FactoryName": "GLTFImportFactory",
        "bReplaceExisting": False,
    }] if sources else []}
    import_settings = generated / "import-apollo-background.json"
    import_settings.write_text(json.dumps(settings, indent=2) + "\n")
    print(f"Prepared {len(sources)} mesh imports and {len(placements)} placements; "
          f"skipped {len(skipped)} unresolved component references")
    print(f"Import settings: {import_settings}")
    print(f"Placement manifest: {manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
