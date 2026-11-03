# Game mode broad recovery 69

Uncounted, unvalidated drafts only. No proposed manifest entries: none of these
bodies is source complete. `inline_rough.cpp` translates the control flow of
three inline exec bodies, keeping unknown calls, field offsets, weak iterator
semantics and virtual slots explicit. In particular, the missing call arguments
in OnEndGameKickPlayers must be recovered from assembly or better decompilation;
the JumpToSafeZonePhase nested iterator/destructor ABI and player fields also
remain unresolved. These drafts are not build inputs.

`SafeZoneInsideChecks.rough.cpp` lifts both phase branches, shared iterator
filtering/cleanup and volume checks. Its null/cast path, object iteration, local
layouts, global overlap and helper effects remain unresolved; it is not a
source-complete implementation. It replaces the initial raw transcription.
All four source windows are under
`porting/generated/decomp_improved/AFortGameModeAthena/` with matching filenames;
the raw equivalents are under `porting/generated/decomp/AFortGameModeAthena/`.
The per-function exec addresses appear in the drafts. The inlined bodies may be
at different native addresses than the exec addresses.

Other class entries are wrappers. Root probed these actual body targets and
`native_bodies.rough.cpp` now lifts all eight, still uncounted:
RemoveAllLootFromWorld -> 0x14195e060;
StopRemovingAllLoot -> 0x141972f90; OnGamePhaseStepChanged -> 0x141953840;
OnCheckForOrphanedServer -> 0x141952900; EnableGameplayMutator ->
0x14193dc90; GetGameplayMutator -> 0x141946800; GetVehicleClassOverride ->
0x141949f60; AddAIClassToReplicationGraph -> 0x1419331a0. Argument widths and
ordering in exec decompilation are not reliable source prototypes. Other
candidates already have behavior artifacts or are thin event wrappers.

Selection checked with bounded `rg -l` against porting/behavior before edits.
Commands: inspected the AGENTS instructions, README, top HANDOFF and master plan;
ranked class dump by `wc -l`; examined candidate source with `head`, `sed` and
`cat`; wrote the files here. No Ghidra, compilation, tests, generator, native
edits, staging or commits. Root owns integration. Last tested baseline and
canonical status are unchanged; these drafts add no counted bodies. Preserve
Luna medium for small reads, Terra 6.1 medium/high for regular bounded work,
Sol 6.1 medium for complex delegated work, fork-none, rotation, and bounded
reads/output.

## Follow-up: eight target probe bodies

Root's serial probe generated `porting/generated/gamemode-broad69/` with
`.c`, `.asm`, and `.json` per address. `native_bodies.rough.cpp` captures the
branches and explicit unresolved operations of all eight targets, with two
mutator targets sharing one rough helper. These are not build inputs, counted
bodies, or source-complete proposals. Check especially callable construction and
unreachable-block warnings in the mutator pair; weak/soft-class cache semantics
and class checks in vehicle override; delegate storage, timer setup and missing
arguments in orphan handling; phase-step argument typing and opaque arrays;
static loot wrapper arguments; and delegate list mutation in AddAI. The emitted
C omits arguments in several calls, so the drafts deliberately do not invent them.
The prior parked leads are now probed, though downstream helper effects remain.

Root probe reported: `[probe] probed 8 addresses in 11.4 s; project files changed:`
`/opt/fortnite/ghidra-projects/fortnite-slim.rep/idata/~index.bak`,
`/opt/fortnite/ghidra-projects/fortnite-slim.rep/idata/~index.dat`,
`/opt/fortnite/ghidra-projects/fortnite-slim.rep/project.prp`,
`/opt/fortnite/ghidra-projects/fortnite-slim.rep/versioned/~index.bak`, and
`/opt/fortnite/ghidra-projects/fortnite-slim.rep/versioned/~index.dat`.
The probe uses a rolled-back transaction; this is the previously observed project
metadata warning and root reports no deliberate project save. This follow-up did
not run Ghidra, tests, compilation, generation or canonical edits.

## Follow-up: safe-zone lift and review corrections

Replaced the raw SafeZoneInsideChecks transcription with a C++-shaped lift in
`SafeZoneInsideChecks.rough.cpp`. It captures both phase paths, iterator filter
and cleanup, volume distance checks, and original null/cast path; `U_*` marks
unresolved operations rather than invented behavior. The original global-object
lookup's exact indexing/invalid-pointer behavior, iterator layout, call ABI,
empty/false target identities, stat and stack instrumentation remain unresolved;
review against the source window before use. No source-completeness is claimed.
PhaseStep now calls `U_OriginalEmptyTarget`, whose address is not established;
this denotes the decompiler's original `Stub_Empty` target, not a fabricated
effect. Root split the mutator sketches into a void EnableGameplayMutator and
value-returning GetGameplayMutator, matching the two native return contracts;
there is no invented enable return value or effect. No tests, compilation,
Ghidra or canonical edits were performed.
