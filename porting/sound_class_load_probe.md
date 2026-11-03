# SoundClass / SoundMix load probe

Question: how many of the 317 cooked SoundClass/SoundMix packages from Fortnite
12.60 load in the UE 4.25 Linux runtime (`-nullrhi -nosound`, headless)?

Result: **316 of 317 load** (188 `SoundClass`, 128 `SoundMix`; no error lines,
exit code 0). 1 fails. Verified packages (`.uasset`/`.uexp`) are copied to
`porting/generated/converted/FortniteGame/Content/...` (ignored dir); the
dashboard leaf `audio.sound_classes` counts them (316/317, basis `local`).

## Method

1. List: every `.uasset` under `FortniteGame` in `/mnt/vibrato/fortnite-extracted-full`
   whose relative path matches `SoundClass|SoundMix` (the regex in
   `porting/progress/refresh_totals.py`): 317 (297 under `Content/Sounds`, 20
   under `Content/Athena`). Stems are unique.
2. Closure: `Cue4Inspect <pkg> <extracted> --loose --save-closure
   --max-packages=2000` (hard imports only). Closure size: median 267, max 269
   (the shared Master/class tree), one package with 1.
3. Copy of the staged build at `/mnt/vibrato/fortnite-work/sound-probe/stage`
   (original untouched); closure files merged no-clobber into `FortniteLinux/`
   and `Engine/`.
4. Probe each package, 16 parallel, 300 s timeout:
   `./FortniteLinux.sh -nullrhi -nosound -unattended
   -FortniteProbeAsset=/Game/<path>/X.X -ExecCmds=Quit -abslog=<log>`.
   Pass = `Fortnite asset probe: ... -> SoundClass|SoundMix`, exit 0, no
   Error/Fatal/Failed-to-load lines. The benign
   `LogFileManager: Warning: Error deleting file ... GameUserSettings.ini`
   (parallel runs racing on the config file; hit one package) is ignored.
5. Promote passes only.

## Failures (1)

| Count | Package | Cause |
|---|---|---|
| 1 | `/Game/Sounds/DefaultAppActivationSoundMixManager` | A Blueprint (`..._C`), not a SoundClass/SoundMix by object class; its native parent `/Script/FortniteGame.FortAppActivationSoundMixManager` is absent from the stage. Matches the regex via its name only. Exit 139, DumpDependencies errors. |

## Scope

Load means package/object deserialization succeeded in the runtime with the
dependency closure present. No audio playback (`-nosound`), no mix/class
behavior, and no check of USoundWave payloads were done.

## Rerun

```sh
python3 -I porting/asset_pipeline/sound_probe.py /mnt/vibrato/fortnite-work/sound-probe
# options: --jobs 16 --timeout 300 --skip-closure --skip-probe --reclassify --no-promote --limit N
```

Outputs: `<work>/results.json`, `<work>/logs/{closure,probe}/`. If the verified
set changes, regenerate the `name_regex` in the `audio.sound_classes` leaf of
`porting/progress/components.json` (exact-stem alternation of passing packages).
