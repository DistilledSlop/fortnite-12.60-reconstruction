# Bounded static-mesh export

`export_meshes.py` converts extracted `FortniteGame/Content/**/SM_*.uasset` packages to GLB with the existing Release Cue4Inspect, using `--loose --export-glb`. It never reads keys, edits the extraction, invokes the UE editor, imports content, or stages a build. GLBs contain geometry only; a successful export does not mean Linux-cooked, visually correct, or runtime-usable content. Discovery is a name heuristic, not a count of all meshes or a dashboard denominator.

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
