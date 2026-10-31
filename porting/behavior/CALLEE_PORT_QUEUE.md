# Callee port queue (unidentified helpers that must be ported as functions)

Written by the second callee-identification pass (see `porting/decompilation_notes.md`, "Second pass 2").
Each entry is a callee from `BLOCKING_CALLEES.md` (rank and blocked count as there) that could not be written as an
engine helper or a one-line accessor. The decision says what to do with it. Addresses are from the slim project
`FortniteClient-Win64-Shipping.runtime.exe`. Decompile and disassembly: `porting/generated/callee_probe/` (git-ignored).
Third pass: rank 1 identified and its callers ported where the rest of the body is clear. The other rows are unchanged.
Fourth pass (see `porting/decompilation_notes.md`, "Fourth pass"): rank 4 `0x1419d3ff0` identified (accessor: `GetWorld()` at vtable `+0x150`, then `UWorld::GameState` at `+0x130`, kept if `IsA AFortGameStateAthena`). Row 7 `0x1406aaec0` stays unnamed (low-medium). `FUN_142c0c240` = `FText::FText()` and `FUN_14079d060` = `FText::operator=`, both high; they block only the `ClientBroadcastOn*` bodies, which the generator cannot yet reference (eight missing `InlineMulticastDelegate` members).

Sixth pass (see `porting/decompilation_notes.md`, "Sixth pass"): row 5 `0x141991bf0` identified (accessor: `CurrentPlaylistInfo.OverridePlaylist`, else `BasePlaylist`, on `AFortGameStateAthena`; HIGH). Row 2 and row 3 are identified in part. `FUN_142af1340` is `FortTeamInfoAthena::StaticClass` (HIGH). `FUN_1407ac640` and `0x140685370` are one lock-counted list family (the element type is unknown). None of rows 2, 3 or 5 can be spliced: the generator has no splice kind for non-reflected helpers, so the 17 callers of `0x1419d3ff0` stay blocked. The table below is unchanged.

Tenth pass (see `porting/decompilation_notes.md`, "Tenth pass"; one small probe run, `0x140684de0`, `0x142a6ee20`, `0x1423def80`, `0x1423def70`): the four rank-2 callees are identified. `0x140684de0` is `TMulticastDelegate<void>::Broadcast()` (no arguments; its list is its argument), `0x142a6ee20` is `AFortPlayerControllerZone::StaticClass`, and `0x1423def80` and `0x1423def70` return `this + 0x28f0` and `this + 0x2908` of the zone controller (no-argument delegates, placeholder names). `OnRep_Kills`, `OnRep_AIKills`, `OnRep_TotalPlayerScore` and `OnRep_TeamKillScore` are ported. Rank 3 `0x142cd3770` is a no-op (profiling only). The callee `0x1419e4f10` (in `OnRep_TeamKillScore`) is `lea rax,[rcx+0x3210]`, identified from the dump bytes. `OnRep_DeathInfo` and the PlayerController callers stay blocked (see the ranking table).

Twelfth pass (see `porting/decompilation_notes.md`, "Twelfth pass"; one probe run over 18 callees without output): the identities
of rows 4 to 8 are recorded there. `0x142414c60` is the owner-as-controller accessor (`Cast` of `Owner` to the class of `0x142a3d9f0`,
HIGH on the body; that class is `AFortPlayerControllerAthena`, MEDIUM-HIGH), `0x14297b320` is `UFortLevelSaveComponent::GetPrivateStaticClass`
(HIGH) and `0x140694550` is a MEDIUM removal of the invocations bound to an object. None of the five bodies is ported: each still has a
blocker that is not a one-line accessor (game-state members `+0x1df0`/`+0x1df8`/`+0x1500`, controller members `+0x2b80` to `+0x2be0`,
the 946-byte `0x141f0a3c0`, the stats structs of `0x141b5d450` and `0x141b6bb40`, the functor of `ClientNotifyTeamLost`). The
small-body scan found no body whose only blocker is one accessor (`IsCreativeModePlaylist` at `0x14291e1c0` is blocked by `0x141a4a6f0`).

Thirteenth pass (vtable slot names and a reflection check; no body ported, manifest unchanged at 437 entries). Tools:
`porting/behavior/build_vtable_slots.py` writes `vtable_slots.json` and `vtable_slots.md`, and `porting/behavior/vtable_lookup.py
<class> <hexoffset>` prints a slot. The map has 20 engine tables (4,474 slots, UObject to UCharacterMovementComponent and
APlayerCameraManager) and 6 game tables (engine part only, 1,628 slots). Checks against the dump's constructor vtables: inline
stub bodies agree in 572 of 656 (87.2%), inherited slots equal their owner's entry in 2,393 of 2,486 (96.3%), game inherited
slots in 1,186 of 1,427 (83.1%; the rest are game overrides). Validated slots: 4,318 (HIGH 3,441, MEDIUM 1,084, LOW 1,577).
Anchors: `UObject` 66 (ProcessEvent) and 42 (GetWorld) are HIGH, and `AFortPlayerStateAthena` 238 (GetPlayerTeam) is HIGH
(dump bytes `mov 0x640(%rcx),%rax; ret`, and PlayerTeam is reflected at 0x640). Not mapped: AFortPlayerPawnAthena (no constructor
store found) and the game-declared virtuals above each game class's engine part (222 for PlayerState, 224 for GameState, 274 for
the pawn, 356 for the controller).

Blocker slots (the receiver is stated where reflection establishes it; a candidate is not a resolution):
- `ServerClearSkydiveLeader` and `0x1419e89a0` (Athena controller). `0x2190` (slot 1074) is game-declared: blocked. `0x700`
  (slot 224) has receiver `PlayerCameraManager` (reflected at the controller `+0x2b0`); the model name there is a void function,
  but the dump body returns a 12-byte value through a hidden pointer, which fits the 8 plus 4 byte copy, so the model index is
  wrong for that class: blocked. `0x590` (slot 178) has receiver `CharacterMovement` (pawn `+0x280`, reached from controller
  `+0x6e0`, `MyFortPawn`); the model name there (`IsMoveInputIgnored`, UPawnMovementComponent) is contradicted by the dump,
  and the AActor candidate `InternalTakeRadialDamage` does not apply to this receiver: blocked.
- `AFortPlayerPawn::HandleInStasis` (receiver not established). `0x710` (slot 226, `DetachFromControllerPendingDestroy`) and
  `0x728` (slot 229, `CreatePlayerInputComponent`) are APawn engine slots, validated by identity (MEDIUM). `0x590`, `0x5b8`,
  `0x420` and `0x50` (slots 178, 183, 132, 10) are AActor or UObject slots (`InternalTakeRadialDamage`, `HasActiveCameraComponent`,
  `OnRep_ReplicatedMovement`, and `PreSaveRoot`, whose stub returns false). Candidates only.
- Callee `0x1419ae980`. `0x150` (slot 42) is `GetWorld` on UObject: resolved (HIGH). `0xaf0` (slot 350) is game-declared on the
  game state and the pawn; on the controller it is `InitInputSystem` (MEDIUM). Blocked on the receiver.
- Row 15 `0x1440c99f0` (receiver not established). `0x468`, `0x498` and `0x4c8` (slots 141, 147, 153) are a bool, an actor getter and
  a void function in the model, so none fits a float-valued use. `0x860` (slot 268, `GetJumpMaxHoldTime`, float, ACharacter) is the
  one candidate that fits. Blocked.

Reflection check of the non-reflected member offsets (`fortnite_game_reflection.json`, class records):
- `AFortGameStateAthena` `+0x1df0` and `+0x1df8` are reflected: `CurrentPlaylistInfo` (`FPlaylistPropertyArray`, struct at `+0x1cd0`,
  size 488) has `BasePlaylist` at struct `+0x120` and `OverridePlaylist` at `+0x128`. They are the accessor of the ported
  `GetCurrentPlaylistAthena` (`0x141991bf0`). This names the member reads of `0x14199e3c0` and `0x1419967f0`; their predicates
  `0x141a4a6f0` and `0x14197ccd0` stay unnamed.
- `AFortGameStateAthena` `+0x1500`, `+0x1510`, `+0x1520` and `+0x1528` are not reflected (the gap between `KillFeedEntry` at `+0x1440`
  and `GameMemberInfoArray` at `+0x15d8`). Still blocked: `0x1419982c0` and its callers.
- `AFortPlayerControllerAthena` `+0x2b80` is `OnSkydiveLeaderChanged` (InlineMulticastDelegate), `+0x2bbc` is
  `SkydiveLeaderManualCameraTime` (float), `+0x2bc4` is `InterpolatedSkydiveFollowerViewRotation` (Rotator, 12 bytes, so `+0x2bcc`
  is its Roll), and `+0x2be0` is `SkydiveLeader` (Object, `OnRep_SkydiveLeader`). All named. `+0x2b90` and `+0x2b98` are not
  reflected and stay unnamed.
- `AFortPlayerController` `+0x1b10` is `CreativeModeProfile` (Object): named (the read in `Server_SetCanEditCreativeIsland`).
- Receivers from reflection: controller `+0x2b0` `PlayerCameraManager`, controller `+0x6e0` `MyFortPawn`, pawn `+0x280`
  `CharacterMovement`.

Bodies unblocked by this pass: none. `ServerClearSkydiveLeader_Implementation` and `0x1419e89a0` still need `0x2190`, the `0x700`
and `0x590` names, and `+0x2b90`/`+0x2b98`. `Server_SetCanEditCreativeIsland_Implementation` now has `+0x1b10` and the playlist
members, but `0x141f0a3c0` (946 bytes) and `0x1423a0aa0` remain. `HandleInStasis` needs the receiver of its calls. Next: fix the
receivers of `HandleInStasis`, `0xaf0` and row 15 from the call sites; then name the game-declared slots (`0x2190` on the controller,
`0xaf0` on the game state and pawn) from a game header or the RPC thunks.

Eighth pass (see `porting/decompilation_notes.md`, "Eighth pass"; research only, no Ghidra run): rows 2, 3 and 5 are identified and their bodies are written as pseudocode (`UpdateScoreForEvent`, `UpdateTotalPlayerScore`, `GetCurrentPlaylistAthena`). `FUN_1407ac640` is `TMulticastDelegate<void(AFortPlayerStateAthena*, int32)>::Broadcast` and `0x140685370` is `FMulticastDelegateBase::CompactInvocationList` (both HIGH); the element is `FDelegateBase`. vfn `+0x770` is `GetPlayerTeam()` (`PlayerTeam` at `+0x640`). `FUN_14070fe00` is `FScalableFloat::GetValueAtLevel` (HIGH, engine). Rows 6, 7 and 9 are engine special members (`FGameplayCueParameters` dtor and copy constructor, `FGameplayTagContainer::operator=`). The ranking after this pass is at the end of the table, before "Not queued". The blocker is now the generator (helper splice kind and non-reflected members), not the callees.

Ninth pass (see `porting/decompilation_notes.md`, "Ninth pass"): rows 2, 3, 4 and 5 are ported as manifest entries. Row 2 is
the helper `AFortPlayerStateAthena::UpdateScoreForEvent`, row 3 the helper `UpdateTotalPlayerScore`, row 4 the static helper
`GetFortGameStateAthena` (in `AFortPlayerStateAthena.cpp`), and row 5 the helper `AFortGameStateAthena::GetCurrentPlaylistAthena`
(the Eighth pass name; the entry `GetActivePlaylist` is renamed, not duplicated). The callee rows stay as the queue record; their
notes name the entry. The nine `OnRep_Num*` stat bodies are ported too. They are spliced and checked in a scratch promote (237
spliced, 0 rejected), not promoted into `native/`. The blocker moves to the build owner (promote and build), and the callers
still blocked are those of the unidentified callees below.

Decisions: **port-fn** = port the callee itself as a C++ function (game logic); **accessor** = a small body that
becomes C++ once one field or virtual slot is named; **struct** = a C++ special member (copy, move, destructor) of a
game struct, written once the struct type is known; **verify** = probably an engine function, check against UE 4.25
source before naming.

| Rank | Address | Blocked | Decision | What it is (evidence) | Still unidentified callees |
|---:|---|---:|---|---|---|
| 1 | `0x1406a9dc0` | 18 (13 alone) | identified (third pass): engine helper, not a port | IDENTIFIED: `TMulticastScriptDelegate<FWeakObjectPtr>::ProcessMulticastDelegate<UObject>` (UE 4.25 ScriptDelegates.h, high). The stop rule above is wrong: the loop continues past skipped entries. The 14 delegate broadcasts that use it are ported (see `decompilation_notes.md`, third pass). | Dispatch over a TArray of 16-byte entries `{FWeakObjectPtr Target @+0, FName Function @+8}`. For each entry: resolve the target (`FWeakObjectPtr_Get`), stop at the first entry whose name is None or whose function is missing (`UObject_FindFunction`), call `Target->ProcessEvent(FindFunctionChecked(name), param)` through vtable `+0x210`, then remove the processed entries (`TArray16_ShrinkIfSparse`, memmove). 677-byte body. The stop rule is medium confidence. Needs: the 16-byte entry type, the UObject vtable index of `ProcessEvent` (`0x210 / 8 = 66`, confirm), and the callers `FUN_1406dcbb0`, `FUN_1406b1a90`. | none (direct callees are identified; `FUN_145803c70` is the stack cookie check) |
| 2 | `0x141a3c2b0` | 7 | ported (ninth pass): helper `UpdateScoreForEvent`, public | IDENTIFIED (eighth pass, HIGH body): `AFortPlayerStateAthena::UpdateScoreForEvent(Event, Count, PreviousPoints, Playlist)`. Scoring loop over the playlist's `ScoringData.ScoreDataList` (`FAthenaScoreData`, 0x68 bytes). Every callee is named: `FUN_14070fe00` = `FScalableFloat::GetValueAtLevel`, `FUN_1406a9dc0` = dynamic broadcast of `PointsAddedToScore` (`+0xf08`). Pseudocode in `decompilation_notes.md`, eighth pass. | none (needs the helper splice kind) |
| 3 | `0x141a3c390` | 7 | ported (ninth pass): helper `UpdateTotalPlayerScore`, public | IDENTIFIED (eighth pass, HIGH body): `AFortPlayerStateAthena::UpdateTotalPlayerScore()`. Sums ten int32 point caches (`+0xe88`, `+0xebc` to `+0xefc`; the AI-kill cache `+0xeb4` is not summed). If `Role == ROLE_Authority` and the delta is positive, adds it to `AFortTeamInfo +0x370` (`PlayerTeam` is a `FortTeamInfoAthena`); stores `TotalPlayerScore` (`+0xf00`); otherwise broadcasts the native list at `+0x1180` with `Broadcast`. vfn `+0x770` = `GetPlayerTeam()` (HIGH); `FUN_142af1340` = `FortTeamInfoAthena::StaticClass`; `FUN_1407ac640` = `Broadcast` (below). | none (needs the non-reflected cache members and the native delegate list) |
| 4 | `0x1419d3ff0` | 7 | ported (ninth pass): static helper `GetFortGameStateAthena` (per class .cpp; the other callers need their own entry) | IDENTIFIED (fourth pass, high): accessor. `GetWorld()` (UObject vtable slot 42, `+0x150`), read `UWorld::GameState` at `+0x130`, return it if `IsA AFortGameStateAthena` (`UStruct::IsChildOfUsingStructArray`). Vtable order checked against the 4.25 headers (ProcessEvent = `+0x210`). Its 17 callers also need rank 2, 3 and 5. | none |
| 5 | `0x141991bf0` | 7 | ported (ninth pass): helper `AFortGameStateAthena::GetCurrentPlaylistAthena`, public (renamed from `GetActivePlaylist`) | `GetCurrentPlaylistAthena`: `CurrentPlaylistInfo.OverridePlaylist ? OverridePlaylist : BasePlaylist` on `AFortGameStateAthena` (`CurrentPlaylistInfo` at `+0x1cd0`, an `FPlaylistPropertyArray`: `BasePlaylist` at `+0x120`, `OverridePlaylist` at `+0x128`). HIGH (asm and reflection). | none |
| 6 | `0x1406ae340` | 3 | struct: compiler-generated, port as `= default` (eighth pass) | Implicit destructor of `FGameplayCueParameters` (184 bytes): frees the two `FGameplayTagContainer` members, then releases the `FGameplayEffectContextHandle` controller. MEDIUM-HIGH. | none |
| 7 | `0x1406aaec0` | 3 | struct: implicit copy constructor, identified (eighth pass) | `FGameplayCueParameters` (`GameplayEffectTypes.h:922`). The constant at `+8` is the vtable `0x1459aede0` of `FGameplayEffectContextHandle` (virtual destructor, so the handle is 24 bytes). HIGH. | none (row 9 is named) |
| 8 | `0x142c64fe0` | 3 | struct | Move assignment: frees the target's buffer at `+0x0`, takes the source's pointer and count (`+0x0`, `+0x8`, `+0xc`) and zeroes the source. Also moves a second member at `+0x10`. | none (struct type) |
| 9 | `0x143e8a690` | 3 | engine: `FGameplayTagContainer::operator=` (eighth pass, corrected) | Copy-assign of two `TArray<FGameplayTag>` (8-byte elements): `GameplayTags` at `+0`, `ParentTags` at `+0x10`. Not a generic TArray copy (the third pass said so). HIGH (shape); 1252 call references, engine-wide. | `FUN_1406d9a30` (TArray resize, engine) |
| 10 | `0x143e8a780` | 2 | struct | Copy assignment of a struct with an owned buffer at `+0x10` (size not decoded): frees the old buffer (`FMemory_Free`), then copies the pointer and count pairs. Same family as rows 6 to 9. | none |
| 11 | `0x140f95ae0` | 2 | accessor | Calls vtable `+0x150` on the argument. If the pointer at `+0x188` of the result is null, returns the qword at `+0x320`; otherwise returns the qword at `+0xc0` of that pointer. Same shape as row 4. | virtual `+0x150` |
| 12 | `0x1420ee770` | 2 | accessor | Copies a 3-float vector from the argument into two members (`+0x22c` and `+0x1e2c`), then sets bit `0x8` of byte `+0x1e38` and bit `0x40` of byte `+0x38b`. A setter of 57 bytes, portable once the members are named. | field names |
| 13 | `0x1419ae980` | 2 | port-fn | Location and state colour helper (256 bytes). Calls `FUN_14468fb70`, `FUN_142a3d9f0`, `FUN_141ed6bb0`, `FUN_141edc1f0` and vtable `+0xaf0` and `+0x150`. Called by `AFortGameStateAthena::SetLocationColorInState`. | `FUN_14468fb70`, `FUN_142a3d9f0`, `FUN_141ed6bb0`, `FUN_141edc1f0`, virtual `+0xaf0` |
| 14 | `0x1419c3880` | 2 | port-fn | Probably `CanPlayerCreateInCurrentVolume` (guess from the first pass; not confirmed). 306 bytes, four vtable calls, calls `FUN_141c74820`, `FUN_141c7d420`, `FUN_14297b320`. Called from `ServerEndMinigame` and `ServerRestartMinigame`. | `FUN_141c74820`, `FUN_141c7d420`, `FUN_14297b320`, `FUN_14237f3d0`, `FUN_141eeab20` |
| 15 | `0x1440c99f0` | 2 | port-fn | Float computation: clamps two float inputs against zero and against each other, then calls `FUN_1440b73d0` and `FUN_14443cb90`, with four vtable calls (`+0x468`, `+0x498`, `+0x4c8`, `+0x860`). Needs the slot names. | `FUN_1440b73d0`, `FUN_14443cb90`, virtual slots |
| 16 | `0x144621fb0` | 2 | verify | Handle lookup: index in the low 24 bits of a 64-bit handle, element stride `0x90`, sparse index read from `+0x28` when set, otherwise the inline array at `+0x18`. Probably `FTimerManager::FindTimer`. Verify in `Engine/Private/TimerManager.cpp` before naming. | none |
| 17 | `0x141952190` | 1 (878 bytes) | port-fn | AFortGameModeAthena aircraft drop-zone handling. Strings: `FailedToLoad`, `NetworkErrors`, `Failed to load before game started.`. Nine indirect calls. Port as a whole function after the callees below. | `FUN_141991bf0` (row 5), `FUN_14468fb70`, `FUN_141990ec0`, `FUN_141695900`, `FUN_140fc7960`, `FUN_141971aa0`, `FUN_141971930` |

## Ranking after the eighth pass (rows 2, 3, 8, 9 and 10 updated in the tenth pass)

Ordered by bodies unblocked. Bodies are the OnRep and RPC functions that call `0x1419d3ff0`, the rows above, or the
unnamed callees listed here (caller table in `decompilation_notes.md`, eighth pass).

| Order | Item | Bodies it unblocks | Status | Next action |
|---:|---|---|---|---|
| 1 | Generator: helper splice kind, plus non-reflected members (int32 caches at `+0xe88`, `+0xeb4`, `+0xebc` to `+0xefc`; four `TMulticastDelegate<void, AFortPlayerStateAthena*, int32>` at `+0x1120`, `+0x1150`, `+0x1180`, `+0x11b0`; `AFortTeamInfo +0x370`) | 9 OnRep_Num* stat bodies, plus the three helper bodies (rows 2, 3, 5) | done (ninth pass): entries and bodies in the manifest, 237 spliced in scratch, 0 rejected | Build owner: promote into `native/`, build, run both smoke variants. |
| 2 | `0x140684de0` (`TMulticastDelegate<void>::Broadcast()`), `0x142a6ee20` (`AFortPlayerControllerZone::StaticClass`), `0x1423def80` and `0x1423def70` (zone accessors `this + 0x28f0` and `this + 0x2908`) | `OnRep_Kills`, `OnRep_AIKills` | done (tenth pass): identified, bodies ported (`OnRep_Kills` onrep, `OnRep_AIKills` helper); the zone delegates are `member_field` entries with placeholder names | Build owner: promote and build with the rest of the tenth pass. |
| 3 | `0x142cd3770` (stat scope end, gated by `0x1482472e4`) | `OnRep_NumChestsOpened`, `OnRep_NumMinutesAlive` | decided (tenth pass): no-op | Done. The bodies skip the call (profiling only; no game state written). |
| 4 | `FUN_141c74820`, `FUN_141c7d420`, `FUN_140694550`, `FUN_1419c1a40`, `FUN_1419e0370` | `OnClientVolumeManagerReplicated` | probed (twelfth pass): `0x140694550` MEDIUM (invocation removal), `0x1419c1a40` MEDIUM (AddUnique bind), `0x141c7d420` MEDIUM, `0x141c74820` LOW-MEDIUM; still blocked (first call has no recovered argument; `0x144640410`, `0x141999ba0`, `0x141c821a0` unnamed). |
| 5 | `FUN_14199e3c0`, `FUN_141a2c5b0`, `FUN_141f0a3c0`, `FUN_1423a0aa0`, `FUN_142414c60`, `FUN_14297b320` (and row 14 `0x1419c3880`) | `Server_SetCanEditCreativeIsland_Implementation` | probed (twelfth pass): `0x142414c60` HIGH (Cast of Owner), `0x14297b320` HIGH (`UFortLevelSaveComponent` static class), `0x141a2c5b0` MEDIUM (TArray append), `0x14199e3c0` body HIGH and meaning LOW; still blocked by `0x141f0a3c0` (946 bytes, not probed further), `0x1423a0aa0` and `0x1419c3880`. |
| 6 | `FUN_141b5d450`, `FUN_141b6bb40`, `FUN_141e8db50`, `FUN_141ed1800`, `FUN_141ed2350`, `FUN_14463f970` | `ClientSendMatchStatsForPlayer_Implementation` | probed (twelfth pass): `0x141e8db50` MEDIUM (counters), `0x141b5d450` and `0x141b6bb40` stats structs unnamed, `0x141ed1800` LOW (static struct), `0x14463f970` LOW; still blocked. |
| 7 | `FUN_1419982c0`, `FUN_1419e89a0` | `ServerClearSkydiveLeader_Implementation` | probed (twelfth pass): `0x1419982c0` MEDIUM (sparse-array lookup on the game state, members unnamed), `0x1419e89a0` LOW-MEDIUM (skydive state); still blocked. |
| 8 | `FUN_1419922b0`, `FUN_1419967f0`, `FUN_142bad740`, `FUN_142ee3b30`, `FUN_1446278d0`, plus row 11 and row 16 (`0x140684de0` is identified, tenth pass) | `ClientNotifyTeamLost_Implementation` | probed (twelfth pass): `0x142a53870` MEDIUM (ProcessEvent thunk), `0x1419967f0` LOW-MEDIUM, `0x1419922b0` LOW; `0x1446278d0` (1213 bytes) and the functor still blocked. |
| 9 | 75 distinct unnamed callees (tenth pass count; `FUN_140683490` is called 32 times, `FUN_141ed1800` 20 times, `FUN_140680160` and `FUN_1406800e0` 16 times each) | `OnRep_DeathInfo` | low priority; the tenth pass did not probe it (one Ghidra run was limited to the four rank-2 callees) | Last. |
| 10 | `FUN_1419e4f10` (identified, tenth pass: `lea rax,[rcx+0x3210]`, a no-argument delegate of the Athena controller) | `OnRep_TeamKillScore` | done: ported (onrep); the member is `TeamKillScoreChangedNative` (name LOW) | none. |

Not examined in the eighth pass: rows 8 and 10 (`0x142c64fe0`, `0x143e8a780`) and rows 11 to 17.

## Not queued

- `0x1406801c0`, `0x1406801d0`, `0x1406801e0` (identified as `Stub_Empty`, `Stub_ReturnTrue`, `Stub_ReturnFalse`):
  folded identical bodies, no port needed (empty, returns true, returns false).
- `0x142ec7720`, `0x142ec7730`, `0x142ee5dd0`, `0x142ee5e30`, `0x142ee7880`, `0x142d698b0`, the `*_StaticClass` getters and the
  other entries of `helpers.json`: identified, see `helpers.json`.
- `0x14199e430` and `0x14199e4e0` (playlist-info family): they use the pair accessor of row 5 and two checks
  (`FUN_141a49590`, `FUN_141a4b0d0`) that were not examined. Decide them after row 5.

## Reproduce

`porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input <list.json> --out porting/generated/callee_probe/<dir>`
then `porting/.venv/bin/python -I porting/ghidra/classify_callees.py --probe <dir> --out <dir>`. The probe rolls back its
transaction, so the project is not saved.
