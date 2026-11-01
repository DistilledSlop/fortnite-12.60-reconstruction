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

Native `46b4675` and Engine `08ebac023` build for both non-editor targets and
pass fresh default/Athena direct joins against the full staged frontend closure.
Donut now samples its original indexed PC icon and real App2 texture override;
five graph/five owner cases pass, the real owner requires seven shaders with zero
missing, all 19 source records are retained, and 1,290 pixels change. Prior
Locker/PowerToast/Menu probes remain green; all 28 shader payloads independently
validate. Dashboard effort is 20.861%, with all four product gates still open.

Actual RootLayout/LoginScreen still fail in another original material. Gdb proves
MainL is the next declined owner. Its own static-permutation resource and nine
original overrides have now been decoded; all six unique Slate programs map and
validate. Implement its pinned actual-owner bridge, verify completeness and live
overrides, then retry root-widget loading/construction and recover native UI
lifecycle/helpers. Continue through login/lobby/matchmaking and full gameplay.
No interactive root menu or original-client visual parity is verified. Exact
current source/build/runtime receipts and work ownership are in HANDOFF.
