# Repository layout: where things are and where output goes

Read this before creating files. Everything that is not source or documentation goes under the ignored
`porting/generated/`, in one of the standard folders below. Do not create new top-level folders in `generated/`
for one-off runs; use `runs/`, `builds/` or `agents/`.

## Source (tracked)

| Path | What it is |
|---|---|
| `porting/native/` | The Unreal Engine project (`FortniteLinux.uproject`) for both the reconstructed client and the Linux dedicated server. `Source/FortniteGame/Generated/` is generator output (do not hand-edit; change the generator inputs). `Source/FortniteGame/Private/` holds hand-written code, including the server gameplay files `FortAthena*.cpp` (match flow, inventory, health, building, loot, end game) and `FortniteAthenaGameplay.cpp` (game mode hooks). |
| `porting/behavior/`, `porting/generator_policies/`, `porting/server_scope/` | Generator inputs: recovered function bodies (`*.inc`), policies, scope lists. |
| `porting/server/` | Python backend (accounts, MCP profiles, matchmaking, XMPP, content). |
| `porting/server_systems/` | Server-first design cards (D*, R*, E*) and handoffs. The plan and gate table is `porting/SERVER_FIRST_PLAN.md`. |
| `porting/tools/` | Current analysis and run tools (see `tools/README.md`): `vanilla_play_run.sh`, `wait_run.sh`, `build_server.sh`, `rd.py`, `strx.py`, `client_log_scan.py`, `stage_server_content.py`, ... |
| `porting/*.py`, `porting/*.sh` | Older generation, extraction and run scripts. Many import each other by module name, so they stay at this level until they are regrouped with their imports updated. |
| `porting/notes/` | Probe write-ups and analysis notes. Notes that scripts read or write (e.g. `binary_analysis.md`, `native_classes_status.md`) stay in `porting/`. |
| `porting/patches/` | Patches to the engine checkout `/opt/fortnite/UnrealEngine` (regenerate a patch from `git diff` there after an engine edit). |
| `porting/local_client/` | ProjectChrono, the Windows launcher for the original client. |
| `porting/frontend/`, `porting/asset_pipeline/`, `porting/material_pipeline/`, `porting/progress/` | Frontend reconstruction, asset conversion, progress dashboard. |
| `docs/` | Published dashboard and screenshots. |

## Runtime output (ignored, under `porting/generated/`)

| Folder | Contents | Written by |
|---|---|---|
| `runs/<YYYYMMDD>-<label>-<n>/` | One client/server run: `server.log`, `client.log`, `shots/` (screenshots), `meta.json` (git head, server args, actions), `play_run.out`, `runner.out`, `udp.pcap` | `tools/vanilla_play_run.sh` (label from `FN_RUN_LABEL`, default `play-run`) |
| `builds/` | Build logs, `<YYYYMMDD-HHMMSS>-server.log` | `tools/build_server.sh [--stage]` |
| `agents/<task>/` | A delegated worker's `BRIEF.md`, `REPORT.md`, transcript | the orchestrator |
| `linux-stage/` | The staged dedicated server (binary, config, cooked content) that play runs execute | `build_server.sh --stage`, `tools/stage_server_content.py` |
| `wine-prefix/` (default) and `/tmp/fortnite-dxvk-experiment/prefix` (DXVK) | Wine prefixes for the original client | setup scripts |
| `dump/`, `reflection/` | Decrypted client image and extracted reflection data (inputs to most tools; `reflection/struct_blocks.json` is pinned, never regenerate it) | extraction scripts |

Old, unreferenced output is moved (not deleted) to `/mnt/vibrato/fortnite-archive/<folder>-<date>/` with an `INDEX.txt`.
Delete bulky files of finished runs (`server.log` at VeryVerbose net logging is hundreds of MB, `udp.pcap`) once
their findings are recorded; the root disk is small.

## Typical loop (server gameplay work)

```
porting/tools/build_server.sh --stage
FN_RENDERER=dxvk FN_LP_NUM_THREADS=4 FN_SERVER_MAP=/Game/Athena/Apollo/Maps/Apollo_Terrain \
FN_SERVER_URL_OPTS='?RequiredPlayers=1' FN_SERVER_ARGS='-FortniteSkipUnconvertedLegacyShaderMaps -FortniteTolerateNeedLoadDependencies -FortniteSkipEngineClassNetChecksum -FortniteWarmupSeconds=20' \
FN_ACTIONS='0:wait,UI.State.Startup.SubgameSelect,900 20:click,640,592 0:wait,ShouldShowBeginningOfSeasonDialog,600 25:click,1144,473 0:wait,Join.succeeded,60 0:stopif,Join.succeeded 0:key,Escape 4:click,1144,473' \
porting/tools/vanilla_play_run.sh 1800 &
porting/tools/wait_run.sh porting/generated/runs/<run> 'landed, server movement' 900   # returns early if the client crashes
```

The client display is a private Xvfb `:99`; it can be screenshotted or driven from outside with
`DISPLAY=:99 XAUTHORITY=$(ps -eo args | grep -oE '/tmp/xvfb-run\.[A-Za-z0-9]+/Xauthority' | head -1)`.
