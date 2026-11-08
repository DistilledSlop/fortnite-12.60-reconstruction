Engine-driven needed-function trace (Apollo_Terrain + FortGameModeAthena, one client join, ~40 s).
1. engine_vt.py: reads engine vtables (names) from the built FortniteLinuxServer + .debug symbols.
2. override_diff.py: for the five core Athena classes, original vtable slots that differ from the parent's, named via the engine vtable at the same slot.
3. mk_gdb.py: one-shot breakpoints on the engine base implementations of those slots; run_hits.sh runs server under gdb + one client.
A hit means the engine called a virtual that Fortnite overrides, so that override body is needed.
hits_counts.txt: 54 of 111 hit. needed_engine_driven.json: 59 original override bodies behind the hit slots (0 ported).
Scripts keep scratchpad paths in S=...; adjust before reuse. Symbol dumps (nm) are regenerated, not stored.
