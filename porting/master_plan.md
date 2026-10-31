# Master plan to the stated goal

> **Current state and how to resume: see `HANDOFF.md`** (written when the session wound down).

Goal: a UE project rebuildable from source that joins a full reconstructed
server with vanilla parity. `progress/components.json` is the measure; a
milestone is done only with logged evidence. Rules for every wave: at most
three agents at once, one owner for `native/` and the UE build, no keys or
credentials in Git, and the orchestrator reviews and commits.

## Critical path and acceptance gates

These gates describe the rebuilt native client and reconstructed server. Work on
content, protocol and gameplay can proceed in parallel, but each milestone needs
recorded end-to-end evidence. See the current snapshot in `HANDOFF.md` before
using historical instructions below.

1. **Build and frontend (M1).** Keep the client and server build and native join
   reproducible; fix required generated modules and load or rebuild the frontend
   widgets and assets. Record a rebuilt-client launch reaching an interactive main
   menu. The existing Entry/test maps and Wine login screen do not meet this gate.
2. **Lobby (M2).** Implement the frontend login and lobby flow and the backend
   contracts it uses. Locate the client response parser and verify schema and case
   against binary/reflection evidence; complete needed profile, party, presence and
   session behavior. Record a rebuilt client joining and remaining in a lobby on
   the reconstructed services. Current native direct join is a foundation only.
3. **Matchmaking and replication (M3).** Connect ticket, websocket, session and
   beacon flow to the dedicated server. Verify custom serializers, replication
   conditions and layouts in live multi-client tests; offline checksum agreement
   alone does not establish wire parity. Record matchmaking from the lobby through
   server join and stable replicated state on multiple clients.
4. **Playable match (M4).** Load or rebuild Apollo and required streamed levels
   and Blueprints; attribute and port server-authoritative match flow, bus, storm,
   combat, inventory, building, loot, eliminations, AI and match end. Integrate
   skeletal meshes, animations, materials, effects, HUD and audio. Record a full
   multi-client match through its end with sound and visuals working, and compare
   behavior and presentation against the supplied 12.60 build.

## Standing constraints

- Storage: `/` has ~100 GB free after the original client moved to the new 1 TB
  drive. `Fortnite/` is now a symlink to `/mnt/vibrato/Fortnite` (checksum-verified
  copy; all scripts keep their paths). Keep large artifacts out of the repo and
  clean only intermediates an agent created. UE source and build output stay on `/`.
- Pak access needs the AES keys. Agents are authorized to use them (user, 2026-10-31). Pass them with
  `--key-file=<path>` (or `FORTNITE_AES_KEY_FILE`); the file stays outside the repository and the keys are never
  committed, printed or logged. See `HANDOFF.md`, "Still to do".
- Source-level identity with Epic's unreleased code is not reachable by
  analysis alone. Parity is judged by behavior and wire format.

## AES key commands

Each adds `--all-keys` for the GUID-keyed paks and `--key-file=<path>` so an agent can run it unattended.

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
  it stopped at the AppES entitlement gate
  (`direct_connect_results.md`). Wire parity is therefore checked offline:
  compute class net checksums and replicated layouts from the recovered
  reflection tables and compare with the native classes, and use the native
  client against the native server for live joins.
- 2026-10-31: moved the 86 GB original client to `/mnt/vibrato/Fortnite` (copy verified
  by rsync checksum, then inspect_install.py, extract_reflection.py and a Wine launch
  were run through the symlink). Both drives benchmark alike (~440-510 MB/s write,
  ~580-790 MB/s read), so speed was not a factor. UE source and build state stay on `/`
  because build tools record absolute paths.
