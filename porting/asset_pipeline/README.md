# Bounded static-mesh export

`export_meshes.py` converts extracted `FortniteGame/Content/**/SM_*.uasset` packages to GLB with the existing Release Cue4Inspect, using `--loose --export-glb`. It never reads keys, edits the extraction, invokes the UE editor, imports content, or stages a build. GLBs contain geometry only; a successful export does not mean Linux-cooked, visually correct, or runtime-usable content. Discovery is a name heuristic, not a count of all meshes or a dashboard denominator. The exact Bridge 014 `S_Elevation_Ground_Z` package is also allowed through explicit `--package` selection (and import manifests), but is never included in prefix discovery. The original loose-package export-class census records `BodySetup,StaticMesh` for it; the exception does not admit other `S_` names.

Build Cue4Inspect first as described in `../mesh_conversion.md`. Default input is `/mnt/vibrato/fortnite-extracted-full`; default output and checkpoint are `../generated/asset_pipeline/` (ignored by git). Output must remain under `porting/generated`. Every job gets a temporary copy of the inspector and an isolated fixed-output directory, so it cannot overwrite other conversions. Each package's final directory retains the full extensionless package path, including duplicate basenames in different folders. Only files listed in the checkpoint are authoritative; older unlisted files may remain after a changed export. Each converter invocation indexes the loose tree and can take tens of seconds.

```sh
python3 -I porting/asset_pipeline/export_meshes.py --dry-run --batch 5
python3 -I porting/asset_pipeline/export_meshes.py --batch 5
python3 -I porting/asset_pipeline/export_meshes.py --batch 5 --retry-failed
python3 -I porting/asset_pipeline/export_meshes.py --package FortniteGame/Content/Environments/Homebase/Meshes/SM_HB_Rock_Cliff
python3 -I porting/asset_pipeline/test_export_meshes.py
```

Defaults: batch 10, one worker, 120-second timeout. Batch limit is 100 and worker limit 32. Repeat `--package` to restrict the selection. Runs take a per-output nonblocking lock; completed jobs are checkpointed atomically. A success is skipped only if all recorded GLBs still match SHA-256 and pass the GLB container/JSON/mesh-position check, the source `.uasset` and present `.uexp`/`.ubulk`/`.uptnl`/`.m.ubulk` SHA-256 hashes match, and the inspector/dependency fingerprints match. The input fingerprint covers the package and its sidecars, not every referenced external package. This is a structural check, not a complete glTF semantic or geometry validation. A missing/corrupt/changed output or input is eligible again; failed entries require `--retry-failed`. An interruption leaves unfinished work eligible. Failures and latest per-package logs are retained in the ignored output; inspect `state.json` and `logs/`. Do not run unbounded batches until the rate, disk capacity, and material limitations are reviewed.

## Reviewable original-path import settings

`import_manifest.py` validates each selected checkpoint against its current input, frozen tool, output hashes and GLB structure before preparing a manifest. It rejects missing, stale or failed entries and any package producing other than a single matching-basename GLB containing exactly one glTF mesh, as required by the matched-path importer patch. Each package becomes a separate import group with its original `/Game` parent directory and `bReplaceExisting: false`, preserving full paths when basenames collide. It does not run the editor or inspect target assets. The importer patch and `-FortniteMatchAssetPath` are required; its simple-as-complex collision, highest-LOD-only geometry, absent original material graphs and unverified cook/runtime remain limitations.

```sh
python3 -I porting/asset_pipeline/import_manifest.py --tool-dir <frozen-tool-directory> --package FortniteGame/Content/UI/Foundation/Lobby/meshes/SM_Lobby_Floor > porting/generated/asset_pipeline/import-review.json
python3 -I porting/asset_pipeline/import_manifest.py --tool-dir <frozen-tool-directory> --package FortniteGame/Content/UI/Foundation/Lobby/meshes/SM_Lobby_Floor --settings-only > porting/generated/asset_pipeline/import-settings.json
python3 -I porting/asset_pipeline/test_import_manifest.py
```

`--settings-only` emits the bare `ImportGroups` JSON suitable for the documented `ImportAssets -importsettings=...` path; the default review output also includes per-package source, target, hash and limitations. A build owner must review the manifest and existing target assets, then perform any import, cook and runtime probe separately. Generation of settings does not demonstrate that UE accepts the multi-group settings or that assets are usable.

For the six direct frontend/lobby mesh imports in `frontend_selection.json`, reproduce the reviewed settings against the frozen tool snapshot used for this batch:

```sh
python3 -I porting/asset_pipeline/import_manifest.py --selection porting/asset_pipeline/frontend_selection.json --tool-dir porting/generated/asset_pipeline/tool-snapshot-frontend --settings-only > porting/generated/asset_pipeline/frontend-evidence/import-settings.json
```

The saved full import-chain reports in `generated/asset_pipeline/frontend-evidence/` establish why each package was selected. They and the snapshot are local, ignored artifacts; recreate a matching snapshot and re-export if the tool changes.

## Skeletal meshes and non-mesh packages

`--prefix SK_` (with a skeletal-capable `--tool-dir` and a separate `--output`, e.g. `generated/asset_pipeline_skeletal`) exports skeletal meshes as skinned GLBs. `Program.cs` pins `FAnimObjectVersion` and disables skeletal ray-tracing data for 12.60, and retries without morph targets when a mesh carries non-finite morph deltas.

Many `SM_`/`SK_`-named packages are not meshes (Blueprints, physics assets, skeletons, animations, control rigs). `classify_nonmesh.py` reads both checkpoints and their logs and writes `nonmesh_packages.json`; `progress/refresh_totals.py` subtracts those from the mesh denominators (`cooked_mesh_exportable`). Rerun it after any bulk export, then `refresh_totals.py`.

## Export-class census and AnimSequence conversion

The asset registry has no AnimSequence rows, so animation packages are found by export class. `Cue4Inspect --scan-export-classes <extracted-root> --classes-out=<tsv> [--workers=N]` walks every loose `.uasset`/`.umap` under the root in parallel (default 16 workers, max 32), reads only each package header (summary, name, import and export maps; exports stay lazy and `.uexp`/`.ubulk` are never opened) and writes sorted `package<TAB>class1,class2,...` rows (distinct export class names; `Class` for a null class index). Unreadable packages are listed on stderr and make the exit code 1. Build it into a scratch tool directory, not `cue4inspect/bin`:

```sh
dotnet build porting/cue4inspect/Cue4Inspect.csproj -c Release -o /mnt/vibrato/fortnite-asset-pipeline/tool-scan
dotnet /mnt/vibrato/fortnite-asset-pipeline/tool-scan/Cue4Inspect.dll --scan-export-classes /mnt/vibrato/fortnite-extracted-full \
    --classes-out=/mnt/vibrato/fortnite-asset-pipeline/export-classes.tsv --workers=24   # ~9 minutes for 454,645 packages
python3 -I porting/asset_pipeline/animation_inventory.py          # writes animation_inventory.json
python3 -I porting/progress/refresh_totals.py                     # animation_sequence_packages
```

`animation_inventory.py` writes the tracked `animation_inventory.json`: per-class package counts for animation-related classes and the sorted list of packages with an AnimSequence export (the `animation_sequence_packages` denominator of the `assets.animation_sequences` dashboard leaf).

`export_anims.py` reuses the `export_meshes.py` checkpoint machinery to run `--loose --export-anim` over that list (from the TSV, or the tracked inventory when the TSV is absent). Defaults: tool `/mnt/vibrato/fortnite-asset-pipeline/tool-scan`, output `generated/asset_pipeline_anim/` whose `anim/` is a symlink to `/mnt/vibrato/fortnite-asset-pipeline/anim`, batch 100, 24 workers (max), 300-second timeout. Each package yields `anim/<package>/<name>.ueanim`, checked for a `UEFORMAT` magic and `UEANIM` identifier (a container check, not a semantic one). The checkpoint is written once per batch; skipping an ok record checks only that its files exist unless `--verify` rehashes inputs and outputs. Failures record the classified `Inspection failed:` reason; retry with `--retry-failed`. It refuses to start a batch with less than `--min-free-gb` (15) free on `/`; exit code 3 means nothing is left to attempt.

```sh
mkdir -p /mnt/vibrato/fortnite-asset-pipeline/anim porting/generated/asset_pipeline_anim
ln -sfn /mnt/vibrato/fortnite-asset-pipeline/anim porting/generated/asset_pipeline_anim/anim
python3 -I porting/asset_pipeline/export_anims.py --dry-run
until python3 -I porting/asset_pipeline/export_anims.py; [ $? -gt 1 ]; do :; done
```
