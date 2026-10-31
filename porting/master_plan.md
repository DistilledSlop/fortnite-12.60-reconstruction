# Master plan to the stated goal

Goal: a UE project rebuildable from source that joins a full reconstructed
server with vanilla parity. `progress/components.json` is the measure; a
milestone is done only with logged evidence. Rules for every wave: at most
three agents at once, one owner for `native/` and the UE build, no keys or
credentials in Git, no EAC/BattlEye/AppES bypass, and the orchestrator reviews
and commits.

## Critical path

1. **Know what the binary gives us** (exe analysis, Ghidra at
   `/opt/fortnite/ghidra-install/ghidra_12.2_PUBLIC`). If the code is readable,
   export class layouts, vtables and reflection tables. If it is packed, stop
   and ask the human before any runtime dump.
2. **Network parity step 0** (version override, MagicHeader, AES/Oodle, key
   delegate). Gate: server logs checksum 683914758 and the native join passes.
3. **Reflection-exact base classes**: FortWorldSettings, FortGameStateAthena,
   FortPlayerStateAthena, FortPlayerControllerAthena, FortPlayerPawnAthena,
   FortAthenaMapInfo. Gate: class checksums match in client and server logs.
4. **Original client joins the server** (direct connect first, then lobby,
   ticket websocket, session, beacons). Gate: Welcome and Join succeeded,
   stays connected.
5. **Cooked Blueprints load**: fix the field-path crashes or rebuild each
   gameplay Blueprint natively. Gate: Apollo loads with its 87 streamed levels.
6. **Gameplay systems** in weight order: Athena framework, weapons, building,
   inventory, abilities, storm, bus and gliding, AI, world managers.
7. **Content**: skeletal meshes and animation, materials, VFX, lighting,
   cinematics.
8. **Audio and frontend**: sound classes, playback backend, CommonUI/UMG
   modules, lobby flow, HUD.
9. **Parity verification**: play a full match against the 12.60 build as the
   reference; record each difference.

## Standing constraints

- Storage: `/` has ~100 GB free after the original client moved to the new 1 TB
  drive. `Fortnite/` is now a symlink to `/mnt/vibrato/Fortnite` (checksum-verified
  copy; all scripts keep their paths). Keep large artifacts out of the repo and
  clean only intermediates an agent created. UE source and build output stay on `/`.
- Pak access needs the AES keys, entered by the human through hidden prompts.
  Agents cannot supply them; work needing them is queued for the human.
- Source-level identity with Epic's unreleased code is not reachable by
  analysis alone. Parity is judged by behavior and wire format.

## Needs the human (AES key via hidden prompt; agents cannot do these)

Queued while the user is away. Each adds `--all-keys` for the GUID-keyed paks.

1. `dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/Frontend --save-closure --max-packages=400 --all-keys`
2. Same for `FortniteGame/Content/Maps/FortniteTransition` (cap 200).
3. Same for `FortniteGame/Content/Maps/UI/Frontend_Lobby_Default` (cap 400).

Why: the Frontend widget graph, 2,171 unseen WidgetBlueprints and pakchunk
1000-1002 are unmeasured without the key (see `ui_audio_census.md`).

## Decisions made autonomously

- 2026-10-30: the supplied `.id0/.id1/.nam` IDA files are untrusted (user); the
  exe `.text` is encrypted on disk, so code is recovered from a read-only
  runtime dump of our own Wine run (user: approved).
- Audio on this host: use `SDL_AUDIODRIVER=dummy` (or `disk` for verification)
  until a real sink exists.
- 2026-10-30: the original exe cannot be used as a handshake oracle through
  the frontend. Without `-AUTH_*` flags it demands a manual sign-in; with them
  it stops at the AppES entitlement gate, which is not bypassed
  (`direct_connect_results.md`). Wire parity is therefore checked offline:
  compute class net checksums and replicated layouts from the recovered
  reflection tables and compare with the native classes, and use the native
  client against the native server for live joins.
- 2026-10-31: moved the 86 GB original client to `/mnt/vibrato/Fortnite` (copy verified
  by rsync checksum, then inspect_install.py, extract_reflection.py and a Wine launch
  were run through the symlink). Both drives benchmark alike (~440-510 MB/s write,
  ~580-790 MB/s read), so speed was not a factor. UE source and build state stay on `/`
  because build tools record absolute paths.
