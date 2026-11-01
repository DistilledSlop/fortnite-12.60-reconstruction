# Master plan to the stated goal

> **Current state and how to resume: see the latest snapshot in `HANDOFF.md`.**

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
   contracts it uses. The client response parser has been located; continue verifying schema and case
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

## Current work (2026-11-01)

Native `907e1dc` with engine `f716e0cb6` builds and passes both joins, 4,617 offline checksums, 251 snippets and 71 backend tests in the earlier regression (separate build/runtime receipts in HANDOFF); the subsequent party-roster session-key change passes 72 backend tests, with delimiter, admission, capacity and error status remaining local policies and no client end-to-end verification. Native FortGameInstance is selected by config; an isolated stage verified Entry, a live instance and direct join, while UI methods remain stubbed. The corrected root hard-import closure is complete and UIManager class loads; actual root widgets remain blocked on original inline shader-map ABI. All 44 candidate frontend DXBC shaders translate to structurally validated SPIR-V with both vkd3d 1.2 and official Wine vkd3d 2.0: 12 modifier diagnostics with 1.2 and none with 2.0. Descriptor set, binding and storage-class inventories are unchanged, but semantics and UE draws remain unverified. Original D3D wrappers now parse with exact framing; 19 carry active resource-table bits with 28 SRV, 18 sampler, 18 texture and zero UAV entries. All 44 translated shaders expose 180 SPIR-V descriptors, all in set 0; no D3D-to-SPIR-V/UE draw mapping has been verified. Exact export-map checks now locate one original inline resource in each of four failing materials: Donut spans `.uexp` 2181..10601, with `M_UI` name bytes at 2225. The local layout misreads the original zero word after the shader-map ID hash as alignment, name length 16 as flags and `M_UI` as frozen size. The strict pre-freeze parser (`2014363`, `18816ad`, `93dfc7f`) covers uniform expressions, empty VT stacks and collections, and compilation scalars in all four resources; the 4.23 texture record corrects Donut’s boundary, so first unknown `.uexp` offsets are 2727, 7410, 7370 and 5905 for Donut, Locker, MenuButton and PowerToast respectively. Packed flags, shader records and the runtime bridge remain unresolved. Original client metadata reports build CL 13315662, distinct from the workspace folder CL 13477524; its network CL is 12896485. Archive Tell 2020 and `.uexp` positions have different coordinates, so the former `+205` origin inference is withdrawn. PowerToast uses parent `M_UI_GradientPanel` as shader identity. Frontend helper probes exposed unresolved call chains and yielded no implementation. Prioritize the genuine material/binding bridge and real root UI draws, then authentic UI lifecycle/missing native helpers and lobby/matchmaking. All four gates remain open.

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

## Historical closure commands

The full extraction and earlier frontend hard-closure scans have since completed; see `extraction_status.md` and `frontend/readiness.md`. These original small-cap commands are retained as historical examples, not current completeness evidence.

Each adds `--all-keys` for the GUID-keyed paks and `--key-file=<path>` so an agent can run it unattended.

1. `dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/Frontend --save-closure --max-packages=400 --all-keys`
2. Same for `FortniteGame/Content/Maps/FortniteTransition` (cap 200).
3. Same for `FortniteGame/Content/Maps/UI/Frontend_Lobby_Default` (cap 400).

The original graph and package-coverage assumptions below these commands predate the full extraction; see the current snapshot for remaining UI gaps.

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
