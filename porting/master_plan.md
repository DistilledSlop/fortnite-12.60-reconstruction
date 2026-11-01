# Master plan to the stated goal

> **Current state and how to resume: see the latest snapshot in `HANDOFF.md`.**

Goal: a UE project rebuildable from source that joins a full reconstructed
server with vanilla parity. `progress/components.json` is the measure; a
milestone is done only with logged evidence. Rules for every wave: at most
three bounded task agents at once, rotating fresh after relevant checkpoints, one owner for `native/` and the UE build, no keys or
credentials in Git, and the orchestrator reviews and commits.

Latest independently verified runtime is native `5d45ea2` / Engine `d7e04d4b6`: Google defaults/combined, both builds, full context checks, retained material/texture/Splash regressions and fresh joins pass. The dashboard credits 126/142 bounded material owners and 41/41 bounded non-Native signatures; all four milestones remain open. First observed load boundaries are Root M_UI_ComicBubble and Login M_TempIcon_Login_Facebook; Native OnBeginIntro remains unimplemented. See the latest HANDOFF.md section for frozen evidence and bounded candidate ownership.

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

The dashboard now separates three frozen frontend source stages and verified generic context runtime from broad manual-zero UI buckets. Same-snapshot effort becomes21.226% (source-only21.172%, originally21.010%), with total1,855 weight preserved across112 leaves. This model correction credits bounded source candidates only; all runtime/integration and product gates remain separate. Source importer rehashes46,166 frozen bindings; runtime importer binds the complete phase3/full strict report/independent checksum audit. All16 accounting/history/material tests pass. See progress/README.md for the allocation and provenance limits.

Social and Media now pass both profiles and independent strict verifiers, original GPU bytes, exact binding/size checks and controls. Phase 2 is frozen and root-audited at 95 sources/six immutable binaries/1,388 artifacts, with two passing builds and joins. Preserve failed phase 1 and older baselines. Original atlas World LODGroup remains an explicit inference; actual UI and original pixel parity remain unfinished.

Generic GetContext and v6 diagnostics are applied as95ff94f. Phase3 passes both builds, full strict original metadata/actual loaded properties/registry/GC/travel verification, retained profiles/Splash and both joins; independent integrity audit validates122 source blobs/1,416 artifacts/18binaryrows. Preserve failed phases1/2. Original25-window constructor replay proves the exact computed flags in the corrected expectation. Versioned strict tools in frontend/context_runtime_probe have23 tests; allfour gates remain false and concrete context methods/authentic startup remain unfinished. Fresh UHT/ELF metadata, actual registry/GC/deferred-travel runs and retained regressions are still required. Concrete FrontEnd/GlobalUI methods and authentic startup integration remain separate.

Port full source CommonUI base/Panel/queue and its real input/settings/global/held-action dependencies to satisfy actual OnBeginIntro, preserving all original dispatch, notification, weak-delegate and tick behavior. Do not add empty Native event stubs. The source-derived base activation and opt-in widget diagnostic are integrated as c06a5cb; a single frozen runtime phase is in progress. Queue/Panel/input composition and full adapters remain pending; earlier candidates stay off workspace. Google named-Texture/ten-mip BC1 material diagnostics now pass in phase2, preserving external bulk, logical versus physical sizes, asset sampler1 and measured residency. Next observed material blockers are Facebook and ComicBubble; authentic UI and original pixel parity remain unfinished.

Use new original raw image `image-125.runtime.bin` SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680` for new decrypted-code proofs; keep all old snapshots unchanged. Root-reviewed recovery in frontend/late_gi_init_control_flow_evidence.md proves full intraprocedural Init, real Init→gated InitializeUI and paired PreLoadMap/PostLoadMap/Shutdown registrations (153 frozen inputs/125 outputs/51 helper windows, fresh replay and8 tests). Complete the implementation-ready source/body closure; do not force phases or replace the larger original Init/player/account behavior with a reduced wrapper. Full lobby/backend, matchmaking/replication and gameplay/visual/audio parity remain required by the four gates.


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
