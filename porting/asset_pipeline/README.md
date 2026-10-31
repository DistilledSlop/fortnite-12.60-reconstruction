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

Defaults: batch 10, one worker, 120-second timeout. Batch limit is 100 and worker limit 2. Repeat `--package` to restrict the selection. Runs take a per-output nonblocking lock; completed jobs are checkpointed atomically. A success is skipped only if all recorded GLBs still match SHA-256 and pass the GLB container/JSON/mesh-position check, the source `.uasset` and present `.uexp`/`.ubulk`/`.uptnl`/`.m.ubulk` SHA-256 hashes match, and the inspector/dependency fingerprints match. The input fingerprint covers the package and its sidecars, not every referenced external package. This is a structural check, not a complete glTF semantic or geometry validation. A missing/corrupt/changed output or input is eligible again; failed entries require `--retry-failed`. An interruption leaves unfinished work eligible. Failures and latest per-package logs are retained in the ignored output; inspect `state.json` and `logs/`. Do not run unbounded batches until the rate, disk capacity, and material limitations are reviewed.
