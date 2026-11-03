# Strict Root gate tools

Wrappers used for the strict Root gate runs (batch 15). Each one hard-codes a batch folder and
`mkdir(exist_ok=False)` outputs, so copy and edit the batch name for a new run.

- `promote_classes.py`: runs `gen_native_classes.py --promote`, restores unchanged mtimes, writes a report to `/tmp/frontend-strict-promotion-N/`.
- `build_client.py`: builds `FortniteLinux` (Development) and writes `porting/generated/frontend-strict-batch-N/results.json`. Requires a clean `porting/native/Source` (commit first).
- `run_root_strict.py`: stages the built ELF and runs the Root attach probe, printing unresolved imports.
