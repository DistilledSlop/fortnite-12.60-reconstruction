# Niagara (NS_) system load probe

Question: how many of the 173 distinct `NS_*` cooked packages from Fortnite
12.60 load in the UE 4.25 Linux runtime (`-nullrhi`, headless)?

Result: **1 of 173 load** (`NS_Backpack_Cyclone`, class `NiagaraSystem`, no
error lines). 172 fail. Only the verified system is placed in
`porting/generated/converted/FortniteGame/Content/...` (ignored dir); presence
there means verified loadable.

## Method

1. List: every `NS_*.uasset` under `FortniteGame/Content` and
   `FortniteGame/Plugins` of `/mnt/vibrato/fortnite-extracted-full` (173).
   Content roots: Athena 70, Effects 45, Gadgets 14, PapayaGameplay plugin 12,
   Hoagie plugin 9, Weapons 6, Characters 6, Animation 4, Accessories 4,
   Environments 2, Items 1.
2. Closure: per system, `Cue4Inspect <pkg> <extracted> --loose --save-closure
   --max-packages=2000 --closure-output=...` (hard imports; no `--follow-soft`).
   Closure size (packages incl. the system and Engine packages): min 2, median
   39, mean 77.3, max 542.
   The tool resolves `--closure-output` relative to its own directory
   (`../../../../generated/frontend-probes`), so a copy of the tool is run from
   `<work>/x/y/z/tool`.
3. Copy of the staged build at `/mnt/vibrato/fortnite-work/niagara-probe/stage`
   (original untouched). Recovered `FortniteGame/{Content,Plugins}` files were
   merged (no-clobber) into `FortniteLinux/`, and recovered `Engine/{Content,
   Plugins}` files (material functions, Niagara plugin defaults, Water) into
   `Engine/`.
4. Probe per system, 16 parallel, 300 s timeout each:
   `./FortniteLinux.sh -nullrhi -nosound -unattended
   -FortniteProbeAsset=/Game/<path>/NS_X.NS_X -ExecCmds=Quit -abslog=<log>`.
   Pass = log has `Fortnite asset probe: ... -> NiagaraSystem`, exit code 0,
   and no Error/Fatal/Failed-to-load lines.
5. Promote passes only.

## Failure reasons (172 systems)

| Count | Root cause | What is missing / implied |
|---|---|---|
| 137 | Fatal in `UMaterial::Serialize -> SerializeInlineShaderMaps -> FShaderMapBase::Serialize` (`CurrentPos + Count <= TotalSize` in `FAsyncArchive::Serialize`, exit 139) | Dependency materials (`DefaultSpriteMaterial`, `M_Sprite_Master`, ...) carry cooked Windows inline shader maps that the stock 4.25 Linux reader cannot parse. Needs material conversion/stripping of the shader map section (or recook), not a missing class. |
| 14 | Fatal in `UStaticMesh::Serialize -> FStaticMeshRenderData -> FPositionVertexBuffer::Serialize` (`SerializedElementSize == 0 \|\| == ElementSize`) | Static mesh render data layout differs (same family as the known mesh stride issue in `mesh_conversion.md`); meshes need re-export/conversion. |
| 12 | `Failed to find object '/PapayaGameplay/...'` | `PapayaGameplay` game-feature plugin content is not mounted in the stage (no `.uplugin`/mount point). |
| 9 | `Failed to find object '/Hoagie/...'` | `Hoagie` game-feature plugin is not mounted. |

Notes:
- No failure was a missing `/Script` class: the stage's `Niagara` module
  (`/Script/Niagara` data interfaces etc.) deserialized the NiagaraSystem and
  `FN_*` NiagaraEffectType assets fine (`FN_PlayerStateMinor__Loop` probed
  alone -> `NiagaraEffectType`, exit 0). The blocker is the material
  dependency graph, not Niagara itself.
- Failures are first-fatal only; a system fixed for materials may still hit
  the mesh or plugin issues. Plugin systems (21) were not retried with
  synthesized `.uplugin` files and their own dependencies likely include
  materials too.
- Missing `/Engine/Functions/...` and `/Niagara/...` default packages were an
  initial 137+ source of failures; resolved by merging recovered Engine
  closure files, which exposed the shader map fatal underneath.
- `-nullrhi` only; no render or shader compile check.

## Rerun

```sh
python3 -I porting/asset_pipeline/niagara_probe.py /mnt/vibrato/fortnite-work/niagara-probe
# options: --jobs 16 --timeout 300 --skip-closure --skip-probe --reclassify --no-promote --limit N
```

Outputs: `<work>/results.json` (per system: class, rc, first error, root
cause, closure size), `<work>/logs/{closure,probe}/`. Do not run it with a work
dir under `porting/native`.
