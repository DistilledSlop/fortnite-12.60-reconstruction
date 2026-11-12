# D0/D5: which server functions are really empty in the client, and how to re-create them

Date: 2026-11-11. This replaces every earlier "stripped" list.

## 1. Method (repeatable)

`python3 -I porting/tools/rpc_sweep.py [name-prefix]` reads
`generated/thunk-table/final.json` (exec thunk → real body for every reflected
native function) and looks at the first bytes of each body:

- `stub_nop` (`ret`), `stub_false`, `stub_true`, `stub_null` = empty;
- `real(N)` = code, N = size of the first .pdata chunk;
- for an empty virtual it also reads the same vtable slot in every subclass
  (vtables written by constructors, from `thunk-table/virtual_targets.json`)
  and reports a real override if one exists.

Outputs, committed:

- `server_scope/server_rpc_sweep.json`: all 359 `Server*` RPCs with verdict.
- `server_scope/native_stub_sweep.json`: every reflected native whose body is
  empty (all prefixes).

An RPC marked `mixed` has `_Validate` = `return true` and a real
`_Implementation`. That is a **real** function.

## 2. Result

| Scope | Total | Real | Empty in client |
|---|---|---|---|
| All reflected natives | 11,407 | 10,836 (real, mixed, inline) | **314** (+18 empty in base with a real subclass override, 252 interface, 5 unknown) |
| `Server*` RPCs | 359 | 259 | **98** (+1 with real override, 1 interface) |

So about 3% of reflected natives are empty, not "most server logic". Of the 98
empty Server RPCs, about 60 are Creative, Save the World, cheat, debug or
broadcast functions that we do not need. **The list that matters for Battle
Royale is the 24 below.**

Limits of the sweep: it covers reflected functions only. C++-only server
functions (no UFUNCTION) are found through the string-name index
(`server_scope/string_names.json`) or by following callers. A real body can
still contain a server-only branch that was compiled out; read it before
trusting it.

## 3. The Battle Royale RPCs that are empty, with re-creation design

Signatures are from `generated/reflection/native_funcs.json`. "Helpers" are
functions that the sweep shows as real code in the client; port those from
their decompile and call them. Designs are mine, from the client-side contract
(what the client sends and what it expects to see replicated back). Confirm
each with a vanilla-client test; the test is listed.

### Inventory and weapons

| RPC | Design | Helpers (real) | Test |
|---|---|---|---|
| `AFortPlayerController::ServerExecuteInventoryItem(FGuid ItemGuid)` | Find the item entry with that guid in `WorldInventory->Inventory.ReplicatedEntries`. If its definition is a weapon/gadget: `Pawn->EquipWeaponDefinition(Def, Guid)`. If it is a building piece or trap item, same call (the client then switches to build mode itself). Ignore unknown guids. | `AFortPawn::EquipWeaponDefinition` 0x1423150b0; `ClientEquipItem` 0x1423713f0 | press 1–5 and the build keys: the held weapon changes and other clients see it (`CurrentWeapon` replicates) |
| `ServerAttemptInventoryDrop(FGuid ItemGuid, int Count, bool bTrash)` | Find entry; reject if the item definition has `bCanBeDropped == false` (pickaxe, building pieces). Clamp Count to the stack. Remove Count from the entry (remove the entry at 0, mark the fast array dirty). Unless bTrash: spawn an `AFortPickupAthena` at the pawn, fill `PrimaryPickupItemEntry` (definition, count, LoadedAmmo), call `TossPickup` forward of the pawn. | `K2_RemoveItemFromPlayerByGuid` 0x141efa680; `AFortPickup::TossPickup` 0x14203cda0; `RemoveItemFromQuickBars` 0x1423a83a0 | drop a weapon: it leaves the quickbar and a pickup appears |
| `ServerCombineInventoryItems(FGuid Source, FGuid Target)` | Both entries must have the same definition and be stackable: move count up to MaxStackSize into Target, remove Source if empty. | – | stack two partial ammo/material stacks |
| `AFortPlayerPawn::ServerHandlePickup(AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)` | Reject if `Pickup->bPickedUp` or too far (> about 2 × pickup radius; get the radius from the pawn's pickup component). Set `Pickup->PickupLocationData` (PickupTarget = pawn, FlyTime, StartDirection, ItemOwner = pawn), set `bPickedUp = true`, call the OnReps / force net update: the client plays the fly-to-player effect. After FlyTime: add the item to the inventory (stack if possible, else first free quickbar slot; if the bar is full, swap with the held item and drop the held one), then destroy the pickup. | `AFortPawn::PickUpActor` 0x14233f230; `AddItemToQuickBars` 0x142368650; `UInventoryManagementLibrary::AddItem` 0x142000d10 | walk over ammo (auto pickup) and press E on a weapon |
| `ServerHandlePickupWithSwap(…, FGuid Swap, …)` and `…WithRequestedSwap` | Same, but the item to replace is given: drop that entry first, put the new item in its slot. | same | pick up with full bar while holding a weapon |
| `ServerAcknowledgeDelayedQuickBarAction(TArray<uint32> Ids)` | Remove the ids from the controller's pending list. No-op is acceptable at first. | – | none |

Needed first, not an RPC: **inventory setup at spawn.** Spawn
`AFortInventory` (WorldInventory) owned by the controller, add pickaxe (from the
player's loadout, default `WID_Harvest_Pickaxe_Athena_C_T01`), the five building
piece items and edit tool, wood/stone/metal at 0, and mark the fast array dirty.
Item entries need a fresh `FGuid` each. The list of start items is in the
GameMode's `StartingItems` property (read it from the `Athena_GameMode` Blueprint
default with the asset tools, do not hard-code).

### Building

| RPC | Design | Helpers (real) | Test |
|---|---|---|---|
| `ServerCreateBuildingActor` | **Not empty**: `_Implementation` 0x1423aad00 → 0x14236d1a0 spawns the actor. Port as is. Check that it charges materials; if the charge is compiled out, remove 10 of the selected material after a successful spawn. | – | place a wall |
| `ServerBeginEditingBuildingActor(ABuildingSMActor* B)` | Reject if B's team is not the player's. Set `B->EditingPlayer = PlayerState` (replicated), give/equip the edit tool item (`AFortWeap_EditingTool`), set the tool's `EditActor = B`. | `EquipWeaponDefinition` | press edit on own wall: edit grid appears and stays |
| `ServerEditBuildingActor(B, TSubclassOf<ABuildingSMActor> NewClass, uint8 RotationIterations, bool bMirrored)` | Reject if `B->EditingPlayer` is not this player. Replace B with NewClass at the same transform rotated by RotationIterations × 90°, keep health fraction and team, call `SetMirrored(bMirrored)`. Clear EditingPlayer. | `ABuildingSMActor::SetMirrored` 0x141c1bf20; look for `ReplaceBuildingActor` in the name index before writing a new one | edit a window into a wall |
| `ServerEndEditingBuildingActor(B)` | Clear `B->EditingPlayer`, clear the tool's EditActor, re-equip the previous weapon. | – | cancel an edit |
| `ServerRepairBuildingActor(B)` | Cost = ceil(build cost × missing health fraction × 0.75) of B's material; reject if the player lacks it; remove it; call `B->RepairBuilding(Controller, Cost)`. Take the 0.75 from the game data row for repair cost if present (search `AthenaGameData` rows for `Repair`). | `ABuildingSMActor::RepairBuilding` 0x141c18db0 | damage own wall, press repair |
| `ServerOnMaterialSelection(uint8 Type, uint8 Level)` | Store on the controller (current build material). | – | switch material, place wall |
| `ServerUpgradeBuildingActor` | Save the World only. Skip. | | |

### Bus, skydive, revive, misc

| RPC | Design | Helpers (real) | Test |
|---|---|---|---|
| `UFortControllerComponent_Aircraft::ServerAttemptAircraftJump(FRotator ClientRotation)` | Only if GamePhase is Aircraft, the aircraft has entered the drop zone (set by `OnAircraftEnteredDropZone`), and the player is in the aircraft. Spawn/restart the pawn at the aircraft location (GameMode `RestartPlayer` path with the aircraft start), set control rotation to ClientRotation, start skydiving, set `PlayerState->bInAircraft = false` and remove the player from the aircraft's occupant list. | `AFortPlayerPawnAthena::TeleportToSkyDive` 0x141a267c0; `BeginSkydiving` (inline exec, read its thunk) | press jump on the bus |
| forced exit | In `OnAircraftExitedDropZone`: run the same code for everyone still on board. | | stay on the bus to the end |
| `AFortPlayerStateZone::ServerSetInAircraft(bool)` | Set `bInAircraft` and call its OnRep. | – | – |
| `AFortPlayerPawn::ServerReviveFromDBNO(AController* Instigator)` | If pawn is DBNO: clear DBNO state, set health to `Default.DBNO.ReviveHealth` (30), remove the DBNO gameplay effects/tags, credit the reviver. Solo: not reachable. | `AFortPlayerPawn::ReviveFromDBNO` (inline exec) | squads only (later) |
| `AFortPlayerPawnAthena::ServerSuicide(int Placement, bool)` | Kill the pawn through the normal damage/elimination path with death cause Suicide. Used by "return to lobby" in some states; low priority. | `AFortPawn::ForceKill` 0x1423178a0 | – |
| `UAthenaMarkerComponent::ServerAddMapMarker(FFortClientMarkerRequest)` / `ServerRemoveMapMarker` | Build an `FFortWorldMarkerData` from the request (owner = PlayerState, type, location, marked actor) and send it to each team member's marker component (client RPC `ClientAddMarker`), remove likewise. Solo: only the owner. | – | place a map marker |
| `UFortControllerComponent_Interaction::ServerNotifyStartLongUse / EndLongUse(AActor*)` | Start/stop a timed interaction on the actor (chest open channel, revive). First pass: no-op, because `ServerAttemptInteract` (real, 174 bytes) performs the interaction. | – | search a chest |
| `AFortPlayerPawn::ServerSendZiplineState(FZiplinePawnState)` | Copy to the replicated `ZiplineState` and call its OnRep; on jump-off apply the launch velocity from the state. | – | ride a zipline |
| `ServerSpotActor`, `ServerSendSquadFriend`, `ServerRespondToAbandonMatch`, `ServerReportClientFPS`, `ServerUpdatePlayerData`, `ServerSendClientProgressUpdate`, `ServerSetAimbotDetection`, `ServerEnableAnonymousMode` | No-op. Client sends them; nothing visible depends on them. | | |
| `ServerSendLoadoutConfig(TArray<uint8> Loadout, int Seed)` | No-op if cosmetics are taken from the backend (D3 card §4). | | skins visible |
| `UFortKismetLibrary::K2_SpawnPickupInWorld(…)` (not an RPC, but empty, and item Blueprints call it) | Spawn `AFortPickupAthena` at the position, fill the entry, toss if requested. One shared C++ helper `SpawnPickup(World, Def, Count, Loc, bToss)` should serve this, drops, loot and elimination drops. | `TossPickup` | open a chest |

## 4. Empty non-RPC natives that matter (from `native_stub_sweep.json`)

- `AFortAthenaVehicleSpawner::SpawnVehicle`: re-create (spawn the spawner's
  vehicle class at its transform at match start); part of D14.
- `ABuildingGameplayActorSpawnMachine::*` (6) and `…SpawnChip::OnDestroyPickup`:
  reboot vans; not needed for solo.
- `AAIHotSpot` (9), `AFortAthenaVehicle` (6 test hooks): AI and debug; later.
- `UFortGameplayAbility_MedicPack`, `…JumpBoostPack`: Save the World items.

## 5. What this means for the plan

1. The "stripped" bucket for BR is small and now fully listed: inventory
   execute/drop/combine/pickup, edit and repair building, bus jump, revive,
   markers, zipline, `K2_SpawnPickupInWorld`, vehicle spawner, plus inventory
   setup at spawn. Each has a design above.
2. Everything else a worker meets is ported from the decompile. Before
   re-creating anything not in this card, run `rpc_sweep.py` for the name and
   check the string-name index; if a body exists, port it.
3. Order of work for G8 (core loop): inventory setup → ServerExecuteInventoryItem
   → pickup → drop → building create/edit/repair → bus jump. Weapons fire,
   damage and harvesting run through gameplay abilities and `mixed`/`real`
   RPCs, covered in their own card.
