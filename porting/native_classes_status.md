# Native class promotion: status

Scope: 993 classes generated from the reflection tables into `porting/native/Source/FortniteGame/Generated/` (987 in
`/Script/FortniteGame`) and `porting/native/Source/PlayspaceFramework/Generated/` (6 in `/Script/PlayspaceFramework`), plus
106 `UINTERFACE` declarations (a `U<Name>` class and its `I<Name>` native interface, no functions recovered). The
generated set also includes 895 structs, 409 enums and 467 delegates. The list is `porting/native_class_set.txt`
(1,099 entries: 168 from the earlier passes, then batches 1 to 4 of round 3). Everything compiles: `FortniteLinux`,
`FortniteLinuxServer` and `FortniteLinuxEditor` exit 0 on the final sources. The join smoke test passes on the client and
server binaries (see "Build and smoke").

**Wire compatibility: not yet.** 621 of 16,355 properties and 91 of 1,477 replicated properties are skipped, and 73 of
564 RPCs are not emitted. Both peers in our build agree with each other. Against the original client the
`ClassChecksum` differs for each class with a skipped replicated property, and skipped RPCs shift that class's RPC
indices. The offline parity count is in "Offline class checksum parity" (the checksum tool covers the first 168 entries
only, see Next steps).

## Results

| Measure | Round 1 (21) | Round 2 (168) | Round 3 final (1,099 entries) |
|---|---|---|---|
| Classes (`native_classes_index.json`) | 21 | 168 | 993 |
| Interfaces (`UINTERFACE`, no functions) | 0 | 0 | 106 |
| Properties emitted / total | 1,220 / 1,721 | 2,657 / 3,424 | 15,734 / 16,355 |
| Replicated emitted / total | 349 / 445 | 537 / 658 | 1,386 / 1,477 |
| RPCs emitted / reflection | 271 / 376 | 331 / 428 | 491 / 564 |
| Structs / enums / delegates emitted | 71 / 56 / 118 | 158 / 110 / 157 | 895 / 409 / 467 |
| Skipped properties (replicated) | 501 (96) | 767 (121) | 621 (91) |
| Skipped RPCs | 105 | 97 | 73 |

### Round 3 batches

| Batch | Added | Entries | UBT builds (client+server) | Smoke |
|---|---|---|---|---|
| 1 | 299 classes (ancestor closure of skip-referenced classes, then largest) | 467 | 3 failed (UHT stub clash, UHT sparse delegate, C++ stub parent), 1 green (174.6 s) | pass |
| 2 | 300 classes, same rule; hand-written classes excluded | 767 | 1 failed (C++, missing Build.cs modules), 1 green (153.7 s) | pass |
| 3 | 106 interfaces + 195 classes unblocked by plugin modules and the largest remaining | 1,068 | 1 green (208.5 s) | pass (needs `LD_LIBRARY_PATH` for libcef, see below) |
| 4 | 31 classes unblocked by ReplicationGraph and Gauntlet (test controllers) | 1,099 | 1 green (174.9 s) | pass |

The editor target failed twice on link errors after batch 3 (GameplayTasks, ClothingSystemRuntimeInterface,
OnlineSubsystem missing from `FortniteGame.Build.cs`) and passed after the module fix (40 s incremental), and again after
batch 4 (119.6 s).

## Regenerate

```sh
porting/promote_native_classes.sh
```

The script runs `resolve_class_refs.py` (class reference map), then `gen_native_classes.py --promote`. The promote mode
checks the closure in `porting/native_class_set.txt` and refuses any name listed in
`porting/native_class_blocklist.txt`. It checks every module that an emitted include comes from against
`FortniteGame.Build.cs` and writes `porting/generated/reflection/required_modules.json`, and it stops before writing any
file when a module is missing. The usable-module rule (`ModuleRegistry`): a module is usable when it is an engine
`Source/Runtime` module, a module of a plugin enabled in `porting/native/FortniteLinux.uproject`, or a project module.
A class whose ancestor is in an unusable module is blocked, not generated.

## Module path decision

The class path is part of the wire format, so the package matters. The package of each class comes from
`class_parent_map.json` (the `FPackageParams` record that each block points to), not from the parent's module. The 987
FortniteGame classes live in `FortniteGame`, the 6 PlayspaceFramework classes in `PlayspaceFramework`. Parents may come
from any usable module (Engine, CoreUObject, GameplayAbilities, UMG, AIModule, NavigationSystem, Lobby, Party,
ReplicationGraph, Gauntlet, OnlineSubsystemUtils, Water, MediaAssets, Landscape, and others). The UHT rule that a parent
must be a `UCLASS` of a parsed module is met by the Build.cs dependency, so each one is listed there.

## Naming

UHT keeps the `A`/`U` prefix in the C++ name, but UObject names drop it. So `AFortPawn` is `/Script/FortniteGame.FortPawn`.
Clash checks between hand-written code and generated code compare the stripped name. `gen_stubs.py` `GENERATED_NATIVE`
lists every generated name without its prefix letter, so a regeneration does not stub them again.

## Classes

The per-class table of the 168-class pass was removed: it no longer matches the set. Per-class emitted and skipped
counts for all 993 classes are in `porting/generated/reflection/native_classes_index.json` (git-ignored), and skipped
records in `skipped_properties.json`.

## Blocked and not generated (blocklist, 238 entries)

`porting/native_class_blocklist.txt` (`ClassName<TAB>reason`). Counts by reason:

| Reason | Classes |
|---|---|
| Ancestor in `/Script/McpProfileSys` (module not in the engine tree) | 142 |
| Ancestor in `/Script/ModularGameplay` (module not in the engine tree) | 56 |
| Ancestor in `/Script/BlueprintContext` (module not in the engine tree) | 12 |
| Ancestor in `/Script/MeshNetwork` (module not in the engine tree) | 4 |
| Ancestor in `/Script/CommonUI` (module not in the engine tree) | 4 |
| Ancestor in `/Script/Account` or `/Script/AmbientAudio` (module not in the engine tree) | 2 |
| Ancestor in a plugin the project does not enable: NiagaraAnimNotifies (Niagara) 3, AnimationSharing 2, PhysXVehicles 2, AnimationBudgetAllocator 2, SignificanceManager 1 | 10 |
| UINTERFACE extending another interface (not generated; the generator supports only interfaces whose parent is `UInterface`) | 2 |
| Hand-written in `porting/native/Source` (`AFortGameModeAthena`, `AFortLevelScriptActor`, `AFortStaticMeshActor`, `AFortSkydivingShadowProxy`, `UFortKismetLibrary`, `UFortMetaNavArea_Obstacles`) | 6 |

The 142 McpProfileSys and 56 ModularGameplay classes are not generated: their modules are not in any engine tree, and the
generator does not invent a parent. Interfaces that belong to other packages (Engine, GameplayAbilities and others) are not generated: their package is
not `/Script/FortniteGame`, so they cannot be declared here.

## Skipped items

Skipped properties (621 total, 91 replicated), by category: `class_outside_native_set` 311, `struct_member_unresolved`
155, `delegate_member_unresolved` 87 (all: unresolved parameter types or out/return parameters), `struct_custom_netserialize`
34, `container_element` 18 (includes TMap keys that are generated structs, which have no `GetTypeHash`), `enum_not_available`
8, `interface` 3, `struct_not_available` 3, `class_unnamed` 1, `delegate_sparse_macro` 1 (a sparse multicast delegate
needs its owning class and property name for the UE 4.25 macro, which the reflection does not record).

Skipped RPCs (73 total): `net_flag_without_role` 24 (a function with a net flag but no Server/Client/NetMulticast role),
`class_outside_native_set` 23, `struct_member_unresolved` 14, `struct_custom_netserialize` 8, `enum_not_available` 2,
`struct_not_available` 2.

Custom NetSerialize structs (Fort package): `AthenaPawnReplayData`, `CreateBuildingActorData`,
`FortAIAttributeReplicationProxy`, `FortAISharedRepMovement`, `FortGameplayAbilityTargetData_SingleTargetHit`,
`FortGameplayEffectContext`, `FortRootMotionSource_FallingBoost`, `ReplicatedPhysicsPawnState`, `SharedRepMovement`. A replicated
property or RPC that uses one of them, directly or inside another struct, is skipped (category `struct_custom_netserialize`).

## What blocks wire compatibility

1. **Classes outside the set (`class_outside_native_set`, 311 properties, 23 RPCs).** Mostly the blocked classes above and
   the classes of modules not in the set. This is now the largest block.
2. **Struct member types (`struct_member_unresolved`, 155 properties, 14 RPCs).** 190 promoted types cannot be emitted
   (`type_closure.json`); most are structs that use a blocked class, or an out/return/reference delegate parameter.
3. **Delegate parameters (`delegate_member_unresolved`, 87 properties).** Delegates do not replicate, so they change the
   class layout only.
4. **Interfaces.** A `UINTERFACE` is generated with no functions. Implemented interfaces are not recovered: the reflection
   records only the count (`interface_count`), so no generated class inherits an `I<Name>` class, and the interface
   UFUNCTIONs are not declared.
5. **Replication conditions.** `DOREPLIFETIME` is used without conditions. `COND_*` is not recovered.
6. **Bodies.** No function bodies or constructor logic are recovered. Each emitted RPC `_Implementation` and each RepNotify
   is an empty stub.

## Hand-written code changed

- `porting/native/FortniteLinux.uproject` enables `OnlineSubsystemUtils`, `OnlineFramework`, `ReplicationGraph` and
  `Gauntlet` (on top of `GameplayAbilities`, `Water`, `Landmass`, `GLTFImporter`, `AESHandlerComponent`, `Oodle` and
  `GeometryProcessing`).
- `porting/native/Source/FortniteGame/FortniteGame.Build.cs` lists the modules that generated code needs: `CinematicCamera`,
  `ClientPilot`, `LiveLinkInterface`, `MediaAssets`, `Water`, `AssetRegistry`, `Overlay`, `LevelSequence`,
  `HeadMountedDisplay`, `AnimGraphRuntime`, `Hotfix`, `Lobby`, `OnlineSubsystemUtils`, `Party`, `Rejoin`, `GameplayTasks`,
  `ClothingSystemRuntimeInterface`, `OnlineSubsystem`, `Gauntlet`, `ReplicationGraph` (the list is checked by the generator).
- Stubs removed from `FortniteMapStubs.h` because they are now generated (prefix-insensitive match): 18 in batch 1, 4 more
  (`BuildingLayoutRequirement`, `ClientAnnouncement`, `Mission`, `NavigationGraph`), 15 in batch 2, 11 in batch 3 (including
  the interface stubs). Parents of the remaining stubs that are generated are included (`ABuildingFoundation.h`).
- `FortniteMapClasses.h` lost `UFortNavSystem` (now generated).
- `gen_stubs.py` `GENERATED_NATIVE` lists every generated name without its prefix letter (1,069 names).
- `gen_native_classes.py` changes this round: module registry (usable modules), per-module parent header index,
  module dependency check, blocklist, UINTERFACE emission, sparse delegate skip, TMap struct-key skip, by-value class
  parameters for RPCs, and the TODO check on `TODO_` tokens (not on the class name `..._TODOverride`).
- `AFortGameModeAthena` (hand-written, Apollo stand-in) is blocklisted: a generated class would clash with it.

## Build and smoke

- Builds (UBT invocations, `Build.sh` with both targets in one call for client and server): 13 of 16. Failed: 3 in the
  client and server (UHT stub clash, UHT sparse delegate, C++ stub parent), 1 for batch 2 (C++ module dependencies), 2 for
  the editor (link). Green: client+server after batches 1, 2, 3 and 4; the editor after batch 3 and batch 4. One
  invocation was up-to-date (no compile).
- Compile time for a full client+server build with batch 1 (467 classes): 175 s. Final set (1,099 entries): 175 s
  (batch 4, incremental over batch 3). Editor: 120 s.
- Staging: the two binaries are copied into `porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/`.
  No cook and no `BuildCookRun`.
- Smoke: `porting/server/smoke_join.sh` in direct mode, with `rg` shimmed by `grep -a` on `PATH` and `stdbuf -oL`. Exit 0.
  `ss -lun` shows `UNCONN 0.0.0.0:7777`. The server logs `Local network version checksum: 683914758 (0x28C3B606)`,
  `Join succeeded: vibrato-43A0C0CF8424` and `Athena pawn spawned: class=FortPlayerPawnAthena authority=1`. The client logs
  `Welcomed by server (Level: /Game/Maps/FortniteEmptyDedicated, Game: /Script/FortniteGame.FortGameModeAthena)`.
- **The client now needs `libcef.so` at run time** (the enabled Lobby/Party plugins pull in CEF). The smoke ran with
  `LD_LIBRARY_PATH=/opt/fortnite/UnrealEngine/Engine/Binaries/ThirdParty/CEF3/Linux`. The stage does not carry it yet; a
  staged package must include it.
- Not run: the Apollo branch (`Apollo_Terrain`, probes, terrain and background grid counts).

## Next steps

1. **Interfaces.** Recover the implemented interfaces of each class (the reflection records only `interface_count`; the
   names must come from the class's `FImplementedInterface` table or from the runtime dump), then inherit the `I<Name>`
   class and emit the interface UFUNCTIONs. Until then, `TScriptInterface<I...>` properties are the only interface uses.
2. **Remaining plugins.** Niagara (71 classes via `NiagaraAnimNotifies` and others), AnimationSharing, PhysXVehicles,
   AnimationBudgetAllocator and SignificanceManager are not enabled. Decide whether to enable them (each adds a module
   compile) or keep them blocklisted.
3. **McpProfileSys (142) and ModularGameplay (56).** No source in any engine tree. Needs a source for the plugin, or a
   documented permanent skip. BlueprintContext (12), MeshNetwork (4) and CommonUI (4) are the same case.
4. **Class references (`class_outside_native_set`, 311 properties).** Re-run `resolve_class_refs.py` and widen the set to
   the classes that the remaining skip tokens name (those that are usable and not yet in the set).
5. **Structs (190 not emittable).** Most fail on a blocked class or on an out/return delegate parameter. Support out and
   return delegate parameters (`DECLARE_DYNAMIC_*` with `OutParam` variants) and re-check.
6. **Sparse delegates.** One delegate uses the sparse macro, which needs the owning class and property name. Record
   those in the reflection, then emit the sparse macro.
7. **Replication conditions and custom NetSerialize.** Recover `COND_*` for `DOREPLIFETIME_CONDITION`, and the 9 custom
   NetSerialize struct bodies.
8. **Checksum parity.** `porting/class_checksum.py` covers only the first 168 entries (`GENERATED_COUNT`; it stops at the
   `# Round 3` line). Lift that limit to report parity for all generated classes.
9. **Bodies.** Recover function bodies for the RPC and RepNotify stubs, starting with the Athena join path.
10. **Stage the CEF library** (`libcef.so`) in the staged package so the smoke test does not need `LD_LIBRARY_PATH`.

## Files

Tracked and changed: `porting/gen_native_classes.py`, `porting/gen_stubs.py`, `porting/native_class_set.txt`,
`porting/native_class_blocklist.txt` (new), `porting/promote_native_classes.sh` (unchanged), `porting/native_classes_status.md`
(this file), `porting/native/FortniteLinux.uproject`, `porting/native/Source/FortniteGame/` (`Generated/`,
`FortniteGame.Build.cs`, `Public/FortniteMapStubs.h`, `Public/FortniteMapClasses.h`), `porting/native/Source/PlayspaceFramework/`
(`Generated/`).

Git-ignored outputs: `porting/generated/reflection/fortnite_game_reflection.json`, `class_ref_map.json`,
`skipped_properties.json`, `native_classes_index.json`, `type_closure.json`, `required_modules.json`, `class_checksums.json`.

## Offline class checksum parity

Generated by `porting/class_checksum.py` from the recovered reflection tables (no native build). Method and rules: the module docstring. Output: `porting/generated/reflection/class_checksums.json` (git-ignored).

Validation:

- CRC: `StrCrc32` of the client's handshake string reproduces 683914758 (the value in the client log).
- Engine headers (AActor, APawn, ACharacter, APlayerState, AGameStateBase, AWorldSettings): 43 replicated declarations in the 4.25 headers. Names found in the reflection: 43 (100.0%). Types equal to the GetCPPType string: 41 (95.3%).
- The two type differences are `ACharacter::bIsCrouched` and `bProxyIsJumpForceApplied`: the 4.25 header declares `uint32 ...:1`, UHT would record ElementSize 4 and the type `uint32`, but the shipped binary records ElementSize 1 (`uint8`). The tool follows the binary, which the original client runs (engine skew, not a parse error).
- Bool offsets come from the setter code: 6576 of 6576 bool setters decode, and 6576 of them lie inside their object.

Result for the 168 generated classes (round 1: 21, round 2: 147):

| Set | Classes | Class checksum parity | RPC digest parity | Both |
|---|---|---|---|---|
| Round 1 | 21 | 9 | 9 | 9 |
| Round 1 + 2 | 168 | 132 | 149 | 130 |

Expected = every replicated property and every net RPC of the class and its parents, from the full reflection. Emitted = the same walk with the entries of `skipped_properties.json` removed (the generator output as it stands; it predates the class reference update in `class_ref_map.json`). Emitted structs, emitted UFUNCTION flags and engine parents are taken as the reflection describes them. The 168 classes are the first 168 entries of `native_class_set.txt` (round 1 and round 2); the round 3 entries added after them are not included. Replicated and RPC counts agree with `native_classes_index.json` for all 168 classes.

### Round 1 (21 classes)

| Class | Expected class checksum | Emitted class checksum | RPC digest exp/emit | Skipped replicated (own / with parents) | Skipped RPCs (own) | Parity |
|---|---|---|---|---|---|---|
| AFortGameStateAthena | 0xBB1821D1 | 0xC6B06E69 | 0x8CAB4898 / 0xEC7571E4 | 7 / 9 | 0 | no |
| AFortPlayerStateAthena | 0x2CB514E7 | 0x53C2C91D | 0xF4F4EB17 / 0xD509DC93 | 0 / 5 | 0 | no |
| AFortPlayerControllerAthena | 0xD3039EB6 | 0xB2B03CC1 | 0x4C5BAF79 / 0x6482BBFD | 3 / 7 | 18 | no |
| AFortPlayerPawnAthena | 0xB81068D8 | 0x438881BC | 0xC2421AE2 / 0x26550484 | 1 / 6 | 1 | no |
| AFortWorldSettings | 0xCF424415 | 0xCF424415 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortAthenaMapInfo | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPoiManager | 0x1642E131 | 0x1642E131 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortWorldManager | 0xDDC6C1C9 | 0xDDC6C1C9 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPlayerStateZone | 0x7B96023C | 0xE8AB54DB | 0x1B13F4A2 / 0xCAA5113E | 3 / 5 | 1 | no |
| AFortPlayerState | 0xA769AD42 | 0xE0979D03 | 0xDABCC7D8 / 0x245BA194 | 2 / 2 | 1 | no |
| AFortGameStateZone | 0xD10FAFC9 | 0x8AB12AE4 | 0xC27A04E8 / 0x8174C47A | 2 / 2 | 1 | no |
| AFortGameState_InGame | 0xA9AB3447 | 0xA9AB3447 | 0x08A3D6BC / 0x08A3D6BC | 0 / 0 | 0 | yes |
| AFortGameState | 0xA9AB3447 | 0xA9AB3447 | 0x08A3D6BC / 0x08A3D6BC | 0 / 0 | 0 | yes |
| AFortGameStateBase | 0x1885B732 | 0x1885B732 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPlayerControllerZone | 0x0AE99499 | 0xF3BF6B52 | 0xB77E3A24 / 0x8C896B15 | 0 / 4 | 1 | no |
| AFortPlayerControllerGameplay | 0x6F391AC2 | 0x997AE690 | 0x4B94C819 / 0x5452AF20 | 1 / 4 | 0 | no |
| AFortPlayerController | 0x227E6F47 | 0xFD77BC58 | 0xF5124BB5 / 0x1262DAE6 | 3 / 3 | 8 | no |
| AFortPlayerPawn | 0x82C3A715 | 0x183A5470 | 0x051E4E22 / 0xC3D5CBD1 | 4 / 5 | 1 | no |
| AFortPawn | 0x25B7B51E | 0x938448DA | 0x9661E4A1 / 0x36D933E8 | 1 / 1 | 1 | no |
| APlayspaceGameState | 0x9085A5BA | 0x9085A5BA | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| APlayspacePlayerController | 0x90C200F2 | 0x90C200F2 | 0x9712F24D / 0x9712F24D | 0 / 0 | 0 | yes |

### All 168 generated classes

| Class | Expected class checksum | Emitted class checksum | RPC digest exp/emit | Skipped replicated (own / with parents) | Skipped RPCs (own) | Parity |
|---|---|---|---|---|---|---|
| AFortGameStateAthena | 0xBB1821D1 | 0xC6B06E69 | 0x8CAB4898 / 0xEC7571E4 | 7 / 9 | 0 | no |
| AFortPlayerStateAthena | 0x2CB514E7 | 0x53C2C91D | 0xF4F4EB17 / 0xD509DC93 | 0 / 5 | 0 | no |
| AFortPlayerControllerAthena | 0xD3039EB6 | 0xB2B03CC1 | 0x4C5BAF79 / 0x6482BBFD | 3 / 7 | 18 | no |
| AFortPlayerPawnAthena | 0xB81068D8 | 0x438881BC | 0xC2421AE2 / 0x26550484 | 1 / 6 | 1 | no |
| AFortWorldSettings | 0xCF424415 | 0xCF424415 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortAthenaMapInfo | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPoiManager | 0x1642E131 | 0x1642E131 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortWorldManager | 0xDDC6C1C9 | 0xDDC6C1C9 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPlayerStateZone | 0x7B96023C | 0xE8AB54DB | 0x1B13F4A2 / 0xCAA5113E | 3 / 5 | 1 | no |
| AFortPlayerState | 0xA769AD42 | 0xE0979D03 | 0xDABCC7D8 / 0x245BA194 | 2 / 2 | 1 | no |
| AFortGameStateZone | 0xD10FAFC9 | 0x8AB12AE4 | 0xC27A04E8 / 0x8174C47A | 2 / 2 | 1 | no |
| AFortGameState_InGame | 0xA9AB3447 | 0xA9AB3447 | 0x08A3D6BC / 0x08A3D6BC | 0 / 0 | 0 | yes |
| AFortGameState | 0xA9AB3447 | 0xA9AB3447 | 0x08A3D6BC / 0x08A3D6BC | 0 / 0 | 0 | yes |
| AFortGameStateBase | 0x1885B732 | 0x1885B732 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPlayerControllerZone | 0x0AE99499 | 0xF3BF6B52 | 0xB77E3A24 / 0x8C896B15 | 0 / 4 | 1 | no |
| AFortPlayerControllerGameplay | 0x6F391AC2 | 0x997AE690 | 0x4B94C819 / 0x5452AF20 | 1 / 4 | 0 | no |
| AFortPlayerController | 0x227E6F47 | 0xFD77BC58 | 0xF5124BB5 / 0x1262DAE6 | 3 / 3 | 8 | no |
| AFortPlayerPawn | 0x82C3A715 | 0x183A5470 | 0x051E4E22 / 0xC3D5CBD1 | 4 / 5 | 1 | no |
| AFortPawn | 0x25B7B51E | 0x938448DA | 0x9661E4A1 / 0x36D933E8 | 1 / 1 | 1 | no |
| APlayspaceGameState | 0x9085A5BA | 0x9085A5BA | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| APlayspacePlayerController | 0x90C200F2 | 0x90C200F2 | 0x9712F24D / 0x9712F24D | 0 / 0 | 0 | yes |
| AFortReplaySpectatorPawnBase | 0x001FBF4A | 0x001FBF4A | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| ABattleMapPawnLive | 0x001FBF4A | 0x001FBF4A | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| ABuildingActor | 0x553B984A | 0x553B984A | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| ABuildingGameplayActor | 0x553B984A | 0x553B984A | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| ABuildingGameplayActorPlayerPropAttachment | 0x553B984A | 0x553B984A | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| ABuildingItemCollectorActor | 0xA46D0397 | 0xD5802B80 | 0xF9F17469 / 0x00000000 | 2 / 2 | 1 | no |
| ABuildingItemWeaponUpgradeActor | 0xC15866DD | 0x7FC6CB78 | 0xF9F17469 / 0x00000000 | 0 / 2 | 0 | no |
| ABuildingSMActor | 0xB1B24A76 | 0xB1B24A76 | 0xE21AE7EB / 0xE21AE7EB | 0 / 0 | 0 | yes |
| ABuildingTrap | 0xF42750B4 | 0x42F4761F | 0xE21AE7EB / 0xE21AE7EB | 2 / 2 | 0 | no |
| ABuildingTrapFloor | 0xF42750B4 | 0x42F4761F | 0xE21AE7EB / 0xE21AE7EB | 0 / 2 | 0 | no |
| ABuildingTrapDefender | 0x4E25F5D3 | 0x5D92E99E | 0xE21AE7EB / 0xE21AE7EB | 1 / 3 | 0 | no |
| AFortClientOnlyActor | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| ABuildingWeakSpot | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| ACustomCosmeticModifierBase | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| ACustomCharacterPartModifier | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortAIDirectorDataManager | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortAircraft | 0xA7534C6A | 0xA7534C6A | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortAthenaAircraft | 0x37E3EE4A | 0x37E3EE4A | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortAthenaCreativePortal | 0x5098ADB0 | 0x5098ADB0 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortGameplayMutator | 0xEA9BAAE8 | 0xEA9BAAE8 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortAthenaMutator_AI | 0xEA9BAAE8 | 0xEA9BAAE8 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortBroadcastRemoteClientInfo | 0xE33BACEA | 0x9EBC00A0 | 0x324CA7A7 / 0x74BB2FE3 | 1 / 1 | 1 | no |
| AFortBroadcastSpectatorInfo | 0xAA117068 | 0xAA117068 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortCarriedObject | 0x0C0D08B3 | 0x0C0D08B3 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortClientAnnouncementManager | 0x6B295E7B | 0x6B295E7B | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortCreativePortalManager | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortCreativeRealEstatePlotManager | 0xB4CF7F42 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AFortDamageNumbersActor | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortDecoHelper | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortEmitterCameraLensEffectDirectional | 0x908CE79B | 0x908CE79B | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortFXManager | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortFeedbackManager | 0x811B75D2 | 0x811B75D2 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortGlobalAbilityTargetingActor | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortGlobalEnvironmentAbilityActor | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortInGameMapManager | 0x2AF7F502 | 0x2AF7F502 | 0x23251F9E / 0x23251F9E | 0 / 0 | 0 | yes |
| AFortInventory | 0x40AD00B8 | 0x6F397001 | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AFortLocalizationService | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortMarkActor | 0xA6261B80 | 0xA6261B80 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortMissionManager | 0x04E1368A | 0x90FE5799 | 0x00000000 / 0x00000000 | 2 / 2 | 0 | no |
| AFortMissionState | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortMusicManager | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortObjectiveBase | 0x5D59490A | 0x5D59490A | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPhysicsPawn | 0x49CFB1C9 | 0x49CFB1C9 | 0x030FA6D8 / 0xD2F9B2DB | 0 / 0 | 1 | no |
| AFortPlayerCharm | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPlayerMarkerBase | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPlayerParachute | 0xC01428B0 | 0xBD4DD2CD | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AFortPlayerPet | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPlayerPetRepState | 0xBC38D6E2 | 0x0BE24A86 | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AFortPlayerStartWarmup | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPoiVolume | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortPropertyOverrideReplShared | 0xC3850BAC | 0xE3EC3839 | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AFortQuickBars | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortRemoteControlledPawnAthena | 0x566C77B8 | 0xF8AFA9CF | 0xC2421AE2 / 0x26550484 | 0 / 6 | 0 | no |
| AFortSafeZoneIndicator | 0x5132CA88 | 0x5132CA88 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortSafeZoneStorm | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortScriptedActionManager | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortSkyTube | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortSpecialActorReplicationInfo | 0x1EEAEE32 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AFortSprayDecalInstance | 0x17955AEC | 0xB1B24A76 | 0xE21AE7EB / 0xE21AE7EB | 1 / 1 | 0 | no |
| AFortTeamInfo | 0x701EF77E | 0x6646E834 | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AFortTeamPrivateInfo | 0x72264F92 | 0x9D7627CA | 0x00000000 / 0x00000000 | 2 / 2 | 0 | no |
| AFortThreatParticleActor | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortThreatVisualsManager | 0xB602DEE5 | 0x4C4FB80B | 0x7E1CFFC8 / 0x7E1CFFC8 | 2 / 2 | 0 | no |
| AFortTimeOfDayManager | 0xE9D831D8 | 0xE9D831D8 | 0x43D96817 / 0x43D96817 | 0 / 0 | 0 | yes |
| AFortVehicleManager | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortVisibilityManager | 0xAEA92127 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AGameplayVolume | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| AFortVolume | 0x8E407F53 | 0xB0219D43 | 0x6B6CA2F8 / 0x6B6CA2F8 | 2 / 2 | 0 | no |
| AFortVolumeManager | 0x391E9886 | 0x2A1BEF34 | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
| AFortWindManager | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| APhysicsObject | 0xEBDAE80B | 0x8B2D16A0 | 0xE93CF6F6 / 0x3F29FE7D | 2 / 2 | 1 | no |
| AUnicornDriver | 0xE3EC3839 | 0xE3EC3839 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UAthenaMarkerComponent | 0x62B90470 | 0xCAAAEE3E | 0xEBDF319C / 0xEBDF319C | 1 / 1 | 0 | no |
| UAthenaPlayerMatchReport | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UAthenaResurrectionComponent | 0x646C2045 | 0x646C2045 | 0x4D4A1638 / 0x4D4A1638 | 0 / 0 | 0 | yes |
| UBaseStatManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UBuildingEditModeMetadata | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UBuildingEditModeMetadata_BinaryToggle | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UBuildingEditModeMetadata_Floor | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UBuildingEditModeMetadata_Roof | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UBuildingEditModeMetadata_Stair | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UBuildingEditModeMetadata_Wall | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UBuildingStructuralSupportSystem | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UCreativeQuestManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UCreativeQuickbarComponent | 0xCAAAEE3E | 0xCAAAEE3E | 0xDB3A0620 / 0x5F8AE48B | 0 / 0 | 2 | no |
| UCreativeUserContentManager | 0xCAAAEE3E | 0xCAAAEE3E | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UCustomCharacterPart | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UCustomColorComponent | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UCustomColorSwatch | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UCustomPlayerComponent | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortItem | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortPersistableItem | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortAccountItem | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortAmbientAudioController | 0xCAAAEE3E | 0xCAAAEE3E | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortAnimNotifyState_RootMotionInterrupt | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortCameraMode | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortCharacter | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortCharacterCosmeticManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortCinematicCamera | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortClientBotManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortControllerComponent_LocalizationService | 0xCAAAEE3E | 0xCAAAEE3E | 0xE6D6D700 / 0xE6D6D700 | 0 / 0 | 0 | yes |
| UFortCreativeDeviceButtonComponent | 0xCAAAEE3E | 0xCAAAEE3E | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortCreativeObjectTrackingComponent | 0xCAAAEE3E | 0xCAAAEE3E | 0x69A426B8 / 0x69A426B8 | 0 / 0 | 0 | yes |
| UFortCreativeOptionsBundle | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortCustomizationAssetLoader | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortFeedbackBank | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortFootstepAudioBank | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortGamepadSettings | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortGameplayMessageComponentBase | 0xD8E1EF9B | 0xD8E1EF9B | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortGenericRecord | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortIndicator | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortSlateHUDIndicator | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortHealthBarIndicator | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortWorker | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortHero | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortIndicatorManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortLevelOverlayManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortMiniMapIndicator | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortMiniMapChallengeIndicators | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortMiniMapTeamIndicators | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortMissionGenerator | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortMobileInteractionComponent | 0x9AAE4688 | 0x9AAE4688 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortMovementComp_Character | 0xCAAAEE3E | 0xCAAAEE3E | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortMusicManagerBank | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortMutatorListComponent | 0x44C782CF | 0x44C782CF | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortPawnScriptedBehavior | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortRegisteredPlayerInfo | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortSkinWeightOverrideManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortStatEventManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortStormShieldComponent | 0x0666FFAC | 0x0666FFAC | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortSupplyDropInfo | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortSwimmingAudioBank | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortUnderwaterDamageComponent | 0xCAAAEE3E | 0xCAAAEE3E | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortUserCloudHelperComponent | 0xCAAAEE3E | 0xCAAAEE3E | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortWeaponAnimSet | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortWorldRecord | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortZoneRecord | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UFortZoneTheme | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UHeartbeatManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UPegasusGameEventCollector | 0xCAAAEE3E | 0xCAAAEE3E | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UQuickHealItemPicker | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UStatManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UUnicornAthenaPawnSampler | 0xCAAAEE3E | 0xCAAAEE3E | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UWeaponHitNotifyAudioBank | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UWorldTheme | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| UWorldTileManager | 0x00000000 | 0x00000000 | 0x00000000 / 0x00000000 | 0 / 0 | 0 | yes |
| APlayspace | 0x31385B67 | 0x234408AC | 0x00000000 / 0x00000000 | 1 / 1 | 0 | no |
