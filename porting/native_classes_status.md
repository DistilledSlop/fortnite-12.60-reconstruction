# Native class promotion: status (first pass)

Scope: the eight promotion targets and every ancestor that is not an engine 4.25 class (21 classes), generated from
the reflection tables into `native/Source` and compiled into `FortniteLinux`, `FortniteLinuxServer` and
`FortniteLinuxEditor`. The join smoke test (`porting/server/smoke_join.sh`, direct mode) passes.

**Wire compatibility: not yet.** 171 of 445 replicated properties in the closure are skipped, and 178 RPCs are not
emitted. Both peers in our build agree with each other, but against the original client the `ClassChecksum` differs
for each class with skipped replicated properties, and skipped RPCs shift that class's RPC indices. See "What blocks
wire compatibility".

## Regenerate

```sh
porting/promote_native_classes.sh
```

Inputs are git-ignored under `porting/generated/reflection/`. Outputs are tracked under
`native/Source/FortniteGame/Generated/` and `native/Source/PlayspaceFramework/Generated/`, which are deterministic
(two runs give identical checksums). The closure is `porting/native_class_set.txt`. `--promote` in
`porting/gen_native_classes.py` checks it: each parent is listed or is in `/Script/Engine` or `/Script/CoreUObject`.

## Module path decision

The class path is part of the wire format, so the module name matters. The package of each class comes from
`class_parent_map.json` (the `FPackageParams` record that each block points to). It is not guessed.

- 19 classes are in `/Script/FortniteGame`, the module this project already owns.
- `APlayspaceGameState` and `APlayspacePlayerController` are in `/Script/PlayspaceFramework`. This is a separate
  package in the shipped client, and it is not in the engine tree.
- Decision: a minimal runtime module `native/Source/PlayspaceFramework` (`PlayspaceFramework.Build.cs`, one IMPLEMENT_MODULE
  file) carries those two classes, so their paths are exactly `/Script/PlayspaceFramework.PlayspaceGameState` and
  `/Script/PlayspaceFramework.PlayspacePlayerController`. It is listed in `FortniteLinux.uproject` and is a dependency
  of `FortniteGame`.

## Naming (verified in the logs)

UHT keeps the `A`/`U` prefix in the C++ name, but UObject names drop it. So `AFortPawn` is
`/Script/FortniteGame.FortPawn`. Our server logs `class=FortPlayerPawnAthena` and the client logs
`Welcomed by server (... Game: /Script/FortniteGame.FortGameModeAthena)`. The probe in `FortniteGame.cpp` loads the 21
classes by these paths on both peers (`Native class ...: loaded, super ...`). All 21 load, and each super chain matches
the reflection chain.

## Classes

Columns: emitted / reflection. RPCs are net UFUNCTIONs (Server, Client, NetMulticast). "RepNotify UFUNCTIONs" are
OnRep functions that are emitted because a replicated property names them. Other UFUNCTIONs are not emitted: the RPC
indices come from the class's net fields, and `ClassChecksum` covers replicated properties only
(`CoreNet.cpp` 160-240), so they are not wire-relevant.

| Class | Module (package) | Parent | Properties emitted / reflection | Replicated emitted / reflection | RPCs emitted / reflection | RepNotify UFUNCTIONs | Functions skipped |
|---|---|---|---|---|---|---|---|
| AFortGameStateAthena | /Script/FortniteGame | AFortGameStateZone | 88 / 197 | 59 / 91 | 1 / 1 | 22 | 0 |
| AFortPlayerStateAthena | /Script/FortniteGame | AFortPlayerStateZone | 46 / 66 | 33 / 41 | 7 / 8 | 26 | 1 |
| AFortPlayerControllerAthena | /Script/FortniteGame | AFortPlayerControllerZone | 75 / 161 | 8 / 19 | 72 / 122 | 1 | 50 |
| AFortPlayerPawnAthena | /Script/FortniteGame | AFortPlayerPawn | 56 / 97 | 7 / 14 | 3 / 5 | 4 | 2 |
| AFortWorldSettings | /Script/FortniteGame | AWorldSettings | 23 / 49 | 0 / 0 | 0 / 0 | 0 | 0 |
| AFortAthenaMapInfo | /Script/FortniteGame | AActor | 0 / 35 | 0 / 0 | 0 / 0 | 0 | 0 |
| AFortPoiManager | /Script/FortniteGame | AActor | 9 / 17 | 2 / 4 | 0 / 0 | 2 | 0 |
| AFortWorldManager | /Script/FortniteGame | AInfo | 27 / 50 | 1 / 3 | 0 / 0 | 0 | 0 |
| AFortPlayerStateZone | /Script/FortniteGame | AFortPlayerState | 12 / 27 | 11 / 18 | 1 / 2 | 1 | 1 |
| AFortPlayerState | /Script/FortniteGame | APlayerState | 24 / 55 | 19 / 37 | 2 / 4 | 9 | 2 |
| AFortGameStateZone | /Script/FortniteGame | AFortGameState_InGame | 42 / 74 | 25 / 44 | 1 / 5 | 9 | 4 |
| AFortGameState_InGame | /Script/FortniteGame | AFortGameState | 0 / 0 | 0 / 0 | 0 / 0 | 0 | 0 |
| AFortGameState | /Script/FortniteGame | AFortGameStateBase | 20 / 48 | 13 / 27 | 2 / 7 | 5 | 5 |
| AFortGameStateBase | /Script/FortniteGame | APlayspaceGameState | 0 / 2 | 0 / 2 | 0 / 0 | 0 | 0 |
| AFortPlayerControllerZone | /Script/FortniteGame | AFortPlayerControllerGameplay | 14 / 29 | 1 / 3 | 17 / 26 | 1 | 9 |
| AFortPlayerControllerGameplay | /Script/FortniteGame | AFortPlayerController | 21 / 59 | 11 / 13 | 15 / 15 | 6 | 0 |
| AFortPlayerController | /Script/FortniteGame | APlayspacePlayerController | 152 / 266 | 20 / 31 | 63 / 135 | 5 | 72 |
| AFortPlayerPawn | /Script/FortniteGame | AFortPawn | 145 / 309 | 39 / 65 | 14 / 31 | 20 | 17 |
| AFortPawn | /Script/FortniteGame | ACharacter | 80 / 173 | 25 / 33 | 0 / 15 | 14 | 15 |
| APlayspaceGameState | /Script/PlayspaceFramework | AGameState | 2 / 5 | 0 / 0 | 0 / 0 | 0 | 0 |
| APlayspacePlayerController | /Script/PlayspaceFramework | APlayerController | 1 / 2 | 0 / 0 | 0 / 0 | 0 | 0 |

Totals: 1,721 properties, 837 emitted (884 skipped). 445 replicated, 274 emitted, 171 skipped.
376 RPCs in the reflection, 198 emitted, 178 skipped. The RepNotify UFUNCTIONs emitted total 125. Counts from `porting/generated/reflection/native_classes_index.json`.

## Skipped items (by type)

Skipped properties are recorded per class and per property in `porting/generated/reflection/skipped_properties.json`
(fields: `class`, `property`, `kind`, `category`, `replicated`, `reason`, `tokens`).

| Category | Properties (all) | of which replicated | RPCs |
|---|---|---|---|
| struct (no struct names or layouts recovered) | 281 | 81 | 70 |
| class outside the native set (named, but not in this module) | 232 | 48 | 37 |
| delegate (`DECLARE_*_DELEGATE` type not recovered) | 178 | 0 | 0 |
| class unnamed or unmatched (no name, or the reference thunk did not match) | 148 | 20 | 52 |
| enum (enum names not recovered) | 42 | 22 | 19 |
| interface (`TScriptInterface` names not recovered) | 2 | 0 | 0 |
| container element (`TSet` element) | 1 | 0 | 0 |
| **Total** | **884** | **171** | **178** |

By property kind: Object 257, Struct 226, InlineMulticastDelegate 178, Array 104, Class 40, Enum 26, Byte 16, Map 11,
WeakObject 10, SoftClass 8, SoftObject 5, Interface 2, Set 1.

Other items recorded in the same file: functions with unknown RPC status (0), RepNotify downgrades (0),
non-void RPCs (0), static or const RPCs (0).

Three things make the skip counts look worse than they are, and we fixed or explained them:

- `params` absent meant "zero parameters" for a function with a unique name join, not "unknown". The extractor now
  writes `params: []` for those (4,369 functions).
- Name joins were ambiguous when functions shared a name literal (4,764 functions). The extractor now keeps every
  candidate (`candidates`). Each candidate carries its owner's construct function (`outer_fn`), and the generator
  picks the one whose `outer_fn` is the class's own `Z_Construct_UClass_*` (exactly one for every such function).
- Class references point at a `jmp` thunk to the class's NoRegister function, not at a block. `resolve_class_refs.py`
  follows up to four jmp hops in the runtime dump and matches the NoRegister or Z_Construct function to a block. That
  resolves 2,326 of 2,622 distinct class references. The other 296 stay unresolved.

## What blocks wire compatibility

Measured in a preview run of the same generator (`--promote` with `--source-root porting/generated/native_preview/Source`,
git-ignored; `native/` was not regenerated, so the tables above still describe `native/`). Struct, enum and delegate
types come from `porting/extract_types.py` (`binary_analysis.md` section 9). Counts are from
`porting/generated/reflection/native_classes_index.json` and `type_closure.json`.

Preview totals, before and after the types were added: properties emitted 837 of 1,721 becomes 1,220 of 1,721
(replicated 274 of 445 becomes 349); RPCs emitted 198 of 376 becomes 271 of 376. Of the 884 skipped properties, 383
are emittable now: 219 struct, 122 delegate and 42 enum. Of the 178 skipped RPCs, 73 are emittable now: 54 struct and 19 enum.

1. **Struct layouts and names (recovered).** 4,512 struct tables with names, parents, sizes and property tables. The
   struct-typed property skips go from 281 to 62: 59 have a member type that is unresolved (classes outside the set,
   see item 4), and 3 are replicated structs with a custom NetSerialize whose body is not recovered (`SpawnMachineRepData`,
   `GameMemberInfoArray`, `DelayedQuickBarActions`). RPCs with struct parameters go from 70 to 16: 14 unresolved member
   types, 1 custom NetSerialize (`FastSharedReplication`), 1 plugin struct not in the engine tree (`ProcessMcpProfileUpdate`).
2. **Enums (recovered).** 2,056 enum tables with names and values. Enum-typed property skips go from 42 to 0, and enum RPC
   skips from 19 to 0. The underlying type is assumed `uint8` (the UHT rule).
3. **Delegates (recovered as signatures).** 1,515 signature tables. Delegate-property skips go from 178 to 56, which
   come from 47 delegate types: 39 with a parameter type that is unresolved (41 class references, 7 classes outside the
   set, 2 structs, 1 enum), and 8 with out, reference or return parameters, which are not emitted. Delegates do not
   replicate, so they change the class layout only, not the wire.
4. **Classes outside the set and unnamed classes (not changed).** 232 property and 37 RPC references name classes that are not in
   this closure. 148 property and 52 RPC references are class refs that `resolve_class_refs.py` does not resolve (296 distinct
   targets). Those targets are not class-block functions: they are singleton builders that call a helper and reference no
   class block. These are now the largest block, and they also block the structs and delegates that use them.
5. **Custom NetSerialize in the closure.** Nine Fort-package structs have a custom `NetSerialize` (`AthenaPawnReplayData`,
   `CreateBuildingActorData`, `FortAIAttributeReplicationProxy`, `FortAISharedRepMovement`,
   `FortGameplayAbilityTargetData_SingleTargetHit`, `FortGameplayEffectContext`, `FortRootMotionSource_FallingBoost`,
   `ReplicatedPhysicsPawnState`, `SharedRepMovement`). A replicated property or RPC that uses one of them, directly or
   inside another struct, is skipped (category `struct_custom_netserialize`). Engine structs with a custom serializer
   (for example `FRepMovement`, `FHitResult`, `FGameplayTagContainer`) are emitted by name; their code is the engine's, which
   is assumed to match 4.25.
6. **Lists are partial.** A skipped replicated property changes that class's `ClassChecksum` and the handles after it. 96
   replicated properties are still skipped (was 171). A skipped RPC changes the RPC indices of that class; 105 RPCs are still skipped.
7. **Replication conditions.** `DOREPLIFETIME` is used without conditions. The reflection table does not record
   `COND_*` lifetime conditions, so a conditional property is sent unconditionally.
8. **Bodies.** No function bodies and no constructor logic are recovered. Each emitted RPC `_Implementation` and each
   RepNotify is an empty stub. Constructors and default values are whatever the engine base class sets.

Validation of the recovered types against the 4.25 headers (`porting/validate_types.py`): structs 92.8% exact member
lists, enums 97.2% exact enumerators and values, delegates 99.6% exact parameter counts. The mismatches are mostly
engine skew between the shipped client and the 4.25 tree.

## Hand-written classes changed

- `AFortWorldSettings`, `AFortAthenaMapInfo`, `AFortPoiManager`, `AFortWorldManager` (were in `FortniteMapClasses.h`),
  `AFortPlayerPawn`, `AFortPlayerPawnAthena` (were in `FortniteAthenaGameplay.h`), and the stubs `AFortGameStateAthena`,
  `AFortPlayerController`, `AFortPawn` (were in `FortniteMapStubs.h`) are removed. They are generated now.
- `AFortPlayerPawn` lost its hand-written camera, input bindings (MoveForward, MoveRight, Turn, LookUp, Jump), the
  `FortPlayerPawn ready` log line, and the `-FortniteProbeLook` visual probe. The generated pawn has no input or camera.
  The Apollo pawn probes (`-FortniteProbePawnGround`, `Apollo_Terrain`) were not run in this pass.
- `AFortGameModeAthena` keeps its Apollo stand-in behaviour and still derives from `AGameModeBase`. Its real parent
  chain is `AFortGamePvPBase -> AFortGameModeZone -> AFortGameMode_InGame -> AFortGameMode -> AFortGameModeBase ->
  APlayspaceGameMode -> AGameMode -> AGameModeBase`. None of those are in the closure (the brief scopes this pass to the
  target chains), so it is not promoted. Its `RestartPlayer` logs `Athena pawn spawned: class=... authority=...` (the
  smoke evidence for the Athena pawn).
- `FortniteMapClasses.h/.cpp` keep the classes outside the set. `FortniteMapStubs.h` loses the three stubs
  above. `gen_stubs.py` excludes them (`GENERATED_NATIVE`), so a regeneration does not bring them back.
- `FortnitePortEditor` commandlet includes the generated `AFortPoiManager.h` and `AFortAthenaMapInfo.h`.

## Build and smoke

- Build: `FortniteLinux`, `FortniteLinuxServer`, `FortniteLinuxEditor`, all Development, all exit 0 in the final state.
  Nine build iterations in total: 1-2 failed in UHT (`minimalapi` with `MODULE_API`; `Replicated FString`
  parameters and `Reliable`/`Unreliable` specifiers), 3 failed in C++ (signature spelling), 8 failed on the commandlet
  include. Each one was a generator or include bug, fixed in the generator or the source.
- Staging: the two binaries were copied into `porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/`
  (previous copies kept under `/tmp`). No cook and no `BuildCookRun`.
- Smoke: `porting/server/smoke_join.sh` (direct mode). The first run exited 1: the server's stdout is buffered, and the
  `Join succeeded:` line was not yet in the file when the script checked it. The second run used the `stdbuf -oL`
  equivalent (`LD_PRELOAD=libstdbuf.so _STDBUF_O=L`, environment only, the script is untouched) and exited 0.
  `ss -lun` shows `UNCONN 0.0.0.0:7777` within 2 s. The server logs `Local network version checksum: 683914758`.
  `Join succeeded: vibrato-43A0C0CF8424` and `Athena pawn spawned: class=FortPlayerPawnAthena authority=1` are on the
  server. The client logs `Welcomed by server (Level: /Game/Maps/FortniteEmptyDedicated, Game:
  /Script/FortniteGame.FortGameModeAthena)`, and all 21 classes load on the client too.
- `rg` is not installed. The smoke run used a `grep -a` shim at `/tmp/fn-native-shim/rg` (outside the repo).
- Not run: the Apollo branch (`Apollo_Terrain`, probes, terrain and background grid counts). It depends on the removed pawn
  code and was out of scope.

## Next steps

1. Extract `FStructParams` and `FEnumParams` from the exe, the same way as classes. This clears 281 struct and 42 enum
   properties and 89 RPCs in this closure.
2. Promote the outside-set classes that block replicated properties (list under item 4 above). That is the cheapest
   way to clear the class-outside skips.
3. Promote the GameMode chain (`AFortGameModeAthena` and its parents) and replace the stand-in `AGameModeBase` parent.
4. Recover replication conditions (`COND_*`) from the lifetime data, and add `DOREPLIFETIME_CONDITION` where needed.
5. Check each class against the original `ClassChecksum` (`original_client_net_protocol.md` section 2) once struct
   layouts are in.
6. Recover function bodies for the RPC and RepNotify stubs, starting with the Athena join path.

## Files

Tracked and changed: `porting/gen_native_classes.py`, `porting/extract_reflection.py`, `porting/gen_stubs.py`,
`porting/native_class_set.txt`, `porting/promote_native_classes.sh`, `porting/resolve_class_refs.py`,
`porting/native_classes_status.md` (this file), `porting/native/FortniteLinux.uproject`,
`porting/native/Source/FortniteGame/` (Generated/, FortniteGame.cpp, FortniteGame.Build.cs, FortniteAthenaGameplay.h/.cpp,
FortniteMapClasses.h/.cpp, FortniteMapStubs.h), `porting/native/Source/PlayspaceFramework/` (new),
`porting/native/Source/FortnitePortEditor/Private/FortniteApolloBlueprintCommandlet.cpp`.

Git-ignored outputs: `porting/generated/reflection/fortnite_game_reflection.json` (regenerated, same blocks, new
fields), `class_ref_map.json`, `skipped_properties.json`, `native_classes_index.json`.
