# Full raw extraction status

## What is extracted

- Full tree: `/mnt/vibrato/fortnite-extracted-full` (`Engine/`, `FortniteGame/`).
- 963,427 distinct served paths, 88,263,051,355 bytes (82.2 GiB), 0 missing, 0 size mismatches, 0 extra files, 0 `.partial` leftovers.
- Source: 31 pak files in `Fortnite/FortniteGame/Content/Paks` (28 keys mounted with the primary key, 31 paks in total).
- Each file is the decrypted, decompressed pak payload exactly as CUE4Parse returns it. Nothing is converted.
- Top-level split: `FortniteGame/Content` 955,024 files; `Engine/Content` 4,653; `Engine/Plugins` 1,270; `FortniteGame/Plugins` 2,430; `FortniteGame` 34; `Engine` 16.

## Duplicate paths

- The provider indexes 967,504 pak entries (`indexEntries`). They cover 963,427 distinct paths, so `duplicateEntries` = 4,077 shadowed entries over 4,077 `duplicatePaths` (every duplicated path has exactly two entries).
- The provider winner is the first entry when `provider.Files` is enumerated (descending read order), the same object `provider[path]` returns. Only winners are planned and written. Shadowed entries are counted per pak in the plan (`pakRows[].shadowed`).
- The first full run planned every entry (`provider.Files.Values`), so 4,077 extra writes went to existing targets. The audit below shows that the on-disk bytes are still correct.
- Audit (`--audit-duplicates --repair`, report `porting/generated/extract-full-duplicates.json`): all 4,077 duplicated paths have byte-identical copies (SHA-256), the disk file matches the winner for all 4,077, the provider lookup agrees with the enumeration winner for all, and 0 files needed repair.

## Closure tree vs full tree

- `porting/generated/extracted` (symlink) -> `/mnt/vibrato/fortnite-extracted`: 233,186 files, 19,756,811,477 bytes (19 GB).
- Every closure path exists in the full tree with the same size. Hashes were not compared for the closure tree.
- The full tree has 730,241 more files than the closure tree.

## Audio

- No `.wem`, `.bnk`, `.ogg` or `.wav` files exist in the full tree, so the extraction has no compressed or converted audio.
- Audio is cooked SoundWave bulk. The registry lists 106,362 `SoundWave` assets. Their `.uasset` headers are in the tree, and the audio payload is in `.ubulk`.
- The tree has 49,806 `.ubulk` files (70.0 GB) in total. Only 597 of them (706,864,874 bytes) are under paths containing `Sound`, `Audio` or `Music`.

## Rerun and resume

Run from the repo root with at most 24 workers. The key file is read by path, and its contents are never printed.

```
dotnet porting/generated/cue4inspect-extract/Cue4Inspect.dll Fortnite/FortniteGame/Content/Paks \
  --extract-all /mnt/vibrato/fortnite-extracted-full --all-keys \
  --key-file=/home/vibrato/.config/fortnite-port/aes_keys --workers=24 < /dev/null
```

- Add `--dry-run --plan-out=<json>` to plan without writing.
- Re-running resumes: a planned file is skipped when its target exists with the index size. Writes go to `<target>.partial` and are renamed into place.
- The skip check compares size only. Use `--audit-duplicates --duplicates-out=<json> [--repair]` to check or repair content of duplicated paths.
- The tool stops at the free-space floors (100 GiB on the output filesystem, 12 GiB on `/`).

## Verify

```
dotnet porting/generated/cue4inspect-extract/Cue4Inspect.dll Fortnite/FortniteGame/Content/Paks \
  --extract-all /mnt/vibrato/fortnite-extracted-full --all-keys \
  --key-file=/home/vibrato/.config/fortnite-port/aes_keys --workers=24 \
  --verify --sample=200 --seed=20261031 --summary-out=porting/generated/extract-full-summary.json < /dev/null
```

Verify compares the plan with the tree (missing, size, extra, `.partial`) and re-reads 200 seeded entries from the paks to check byte identity. `ok` is true only when every check passes.

Last run (2026-10-31): `ok: true`, sample 200/200 byte-identical.

## Files

- `porting/generated/extract-full-summary.json`: corrected verify summary (counts, per-pak rows, verification).
- `porting/generated/extract-full-plan.json`: corrected dry-run plan.
- `porting/generated/extract-full-duplicates.json`: duplicate audit.
- `porting/generated/extract-full-run.json`, `porting/generated/extract-full.log`: first (double-planned) run, kept as history.
- Tool: `porting/cue4inspect/Program.cs` (`ExtractAllFiles`, `AuditDuplicates`, `VerifyOutput`); build output `porting/generated/cue4inspect-extract`.
