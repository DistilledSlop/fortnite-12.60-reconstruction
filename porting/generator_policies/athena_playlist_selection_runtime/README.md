# Original playlist selection virtual family

This proposal restores eight complete original bodies: the base and Athena ID/name setters, the inherited name-selection virtual, the game-instance playlist-manager accessor, and both manager lookups. It adds no InitGame call, manager initialization, asset selection, playlist application or game-state mutation.

## Root integration hooks

1. Append the eight `manifest-entries.json` entries to the behavior manifest using their existing policy-local `.inc` paths. They use the regular `helper` admission path, including base virtual declarations and Athena `override` declarations. Keep this family together.
2. Import `source_policy.py` beside the existing generator policies. After the complete promoted class output set is known, call `validate_required_classes(class_names)`; all nine classes in `PARENTS` must be present, including the intermediate `AFortGamePvPBase`, `AFortGameModeZone`, and `AFortGameMode_InGame`.
3. In the final source-policy pass, call `header, cpp = policy.finalize(model, header, cpp)` for every class model. The policy verifies ancestry/package, typed fields, all eight accepted helper signatures and bodies, declarations, emitted bodies, and dependency includes. It then adds only the static log category declaration to the three implementation units. It does not emit helper bodies itself.
4. Review ordinary admission diagnostics and run normal generation checks before promoting/staging. No global generator, manifest or native files have been changed by this proposal.

`SetCurrentPlaylistId(int32)` and `SetCurrentPlaylistName(FName)` correspond to original vslots +0x910/+0x918. `NativeSetCurrentPlaylistName(FName)` is the neutral source name for +0x920: the original Athena +0x918 tail-jumps through that virtual slot. Calling it unqualified preserves dispatch in further derived modes. `GetPlaylistManagerNative` and manager query names are source names rather than recovered debug symbols.

All success and failure paths retain original severity/message text, including the surprising Athena prefix in the base ID-success diagnostic. The new category name is `LogFortPlaylistSelection` because the shipping category symbol at 0x1481348e4 is unresolved. Every source unit has a static category with that common runtime name. Category-specific original configuration is therefore not asserted to be recovered.

The manager accessor uses the same original Fortnite game-instance type assumption (`static_cast`), with the original null-instance return. Callers and lookups deliberately retain the initialized-manager precondition: original query bodies immediately dereference the manager. No fabricated fallback, load, or silently successful missing-manager path is added.

Manager lookups scan `AthenaPlaylists` in order, skip null entries, and return the first exact ID or `FName` match. Failed name lookup logs only when the requested name is not `NAME_None`; the name setter still logs its own failure for `NAME_None`. Failed selection preserves both current fields. Successful selection copies only those fields. Neither setter assigns a game-state playlist or applies playlist settings.

## Validation

Run from `porting/`:

```sh
python3 generator_policies/athena_playlist_selection_runtime/test_source_policy.py
python3 generator_policies/athena_playlist_selection_runtime/test_generator_admission.py
g++ -std=c++14 -Wall -Wextra -Wno-unused-parameter generator_policies/athena_playlist_selection_runtime/behavior_harness.cpp -o generated/agents/athena-playlist-setters5/behavior_harness
generated/agents/athena-playlist-setters5/behavior_harness
```

Six Python guard tests cover the whole family, missing classes, ancestry/member drift, incomplete/rejected/duplicate admission, changed signatures/bodies, and missing/duplicate rendered declarations/definitions/includes. The optional real-generator regression overlays these eight manifest proposals in memory, uses the normal full class context and actual helper/header admission, and exits after all three target class models pass. Outputs stay in this task's ignored artifact directory. The C++ harness compiles the actual `.inc` bodies and exercises unsigned ID validation, dynamic Athena dispatch, the further-derived +0x920 dispatch, numbered names, null entries, first-match order, retained fields on failure, and original diagnostic combinations. It is a focused host harness with typed stand-ins, not a UE build or runtime claim.

Raw decompilation and assembly evidence, an explicit address inventory, and the remaining lifecycle blocker are under `porting/generated/agents/athena-playlist-setters5/`. Captures used an isolated reflink copy of the slim Ghidra project in that ignored directory; only that copy incurred Ghidra metadata writes.
