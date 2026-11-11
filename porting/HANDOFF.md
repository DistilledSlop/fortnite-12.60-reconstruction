# Handoff

## Current focus: vanilla client on our server (2026-11-11, read first)

The working target is the original 12.60 client joining and playing on our backend and dedicated server. The plan, file map, gates and work packages are in `SERVER_FIRST_PLAN.md`. The critical path is the UDP handshake (gate G3), then control channel, map join and a stable session (G4 to G6). Function counts below are a lookup aid, not the measure of progress; see section 3 of the plan.

## Native server port goal (2026-11-10, read first)

All 1,110 server-required native functions in `porting/server_scope/local_model/queue.json` get full bodies, hand-ported by Claude; terra is stopped and "blocked" is not an allowed outcome. Workflow, paths and state: `porting/server_scope/local_model/README.md`. Decompiles for all of them are already captured; do not recapture.


Native function body porting (state, method, rules, remaining work): see `NATIVE_PORTING.md`.

The earlier long handoff and `NATIVE_RECOVERY_HANDOFF.md` were replaced by that file; older checkpoints are in Git history.
For overall plan see `master_plan.md`; for progress see `progress/README.md`.
