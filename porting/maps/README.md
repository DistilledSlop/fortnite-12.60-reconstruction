# Bridge 014 geometry preparation

`bridge014_recipe.py` resolves the original map's 21 static components in streamed-level coordinates, including four Blueprint support CDO meshes. The tracked `bridge014_mesh_selection.json` selects the six distinct packages behind those placements for the existing bounded mesh exporter. The ground package `S_Elevation_Ground_Z` is explicitly allowed by the exporter: the loose-package export-class census records `BodySetup,StaticMesh` for it. Foliage instance transforms and the HLOD proxy are excluded. The streaming transform remains unresolved.

Check that the selection still matches the source reports and produce the level-space recipe:

```sh
python3 porting/maps/bridge014_recipe.py --output porting/generated/streamed_levels/bridge014-recipe.json --check-mesh-selection porting/maps/bridge014_mesh_selection.json
```

To reproduce the six GLB exports, use a built or frozen Cue4Inspect tool directory and a separate output checkpoint. This invokes the exporter, not Unreal's importer:

```sh
python3 - <<'PY'
import json, subprocess
selection = json.load(open('porting/maps/bridge014_mesh_selection.json'))
cmd = ['python3', '-I', 'porting/asset_pipeline/export_meshes.py',
       '--tool-dir', 'porting/generated/asset_pipeline/tool-snapshot-frontend',
       '--output', 'porting/generated/bridge014_meshes', '--batch', '6', '--workers', '5']
for source in selection['sources']:
    for package in source['packages']:
        cmd.extend(['--package', package])
raise SystemExit(subprocess.call(cmd))
PY
python3 -I porting/asset_pipeline/import_manifest.py --selection porting/maps/bridge014_mesh_selection.json --tool-dir porting/generated/asset_pipeline/tool-snapshot-frontend --output porting/generated/bridge014_meshes > porting/generated/bridge014_meshes/import-review.json
```

The local snapshot is ignored and must exist or be replaced with a built tool directory. The manifest validates the checkpoint, source and output hashes, and matched-path import eligibility. It does not import assets. The GLBs provide highest-LOD geometry only: original materials and collision are not reconstructed. No world placement, cook, or runtime validation is implied. See `../asset_pipeline/README.md` for importer prerequisites and limitations.

## Validate and promote the isolated import checkpoint

The local ignored `porting/generated/bridge014_import_checkpoint/Content` is the result of an isolated editor import of the six GLBs. `bridge014_checkpoint_hashes.json` records the selection, six GLBs, and the resulting six mesh and four generated material assets. It is a fingerprint of this particular import, not proof that another editor or import can reproduce identical binary assets. It does not establish material fidelity, collision, cooking, placement, or runtime behavior.

Validate the entire recorded GLB and imported-asset inventories, and check the main project's targets without writing:

```sh
python3 porting/maps/bridge014_checkpoint.py
```

To promote missing files from that exact checkpoint into `porting/native/Content`:

```sh
python3 porting/maps/bridge014_checkpoint.py --promote
```

All hashes and destination conflicts are checked before copying. Identical destination assets are left alone; differing assets, unexpected package sidecars, and symlinks in asset paths stop promotion. Each missing asset is created exclusively, so existing files are never overwritten. An interrupted promotion may leave some copied files; rerun after checking or removing any incomplete file. Use `--checkpoint`, `--glbs`, and `--target` to supply alternate local directories. The checkpoint and GLBs must exist and match the recorded hashes; regenerate and review the import separately if they do not. This workflow promotes the known import and does not invoke the editor.
