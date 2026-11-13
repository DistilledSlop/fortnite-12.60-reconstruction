# Equip, unequip and weapon attachment lifecycle (original 12.60 client image)

Research only. Nothing here was built, run or ported, and no parity is claimed. The user-observed asymmetry
(client 1 sees client 2's pickaxe near vertical) is **not explained** by this document; section 9 lists what the
recovered contract makes measurable.

Scope: `PlayerPawn_Athena_C` (native `AFortPlayerPawnAthena`) holding `B_Athena_Pickaxe_Generic_C`
(native `AFortWeaponPickaxeAthena`). Building tool and ranged weapons are covered only where section 1 says so.

## 0. Evidence classes and sources

Every statement carries one of these tags.

| Tag | Meaning |
|---|---|
| **[B]** | Read from the original image: decompile, disassembly, vtable slot or data bytes. Address given. |
| **[R]** | Recovered reflection metadata: field offsets (`evidence.py field-offset`), `porting/replication_conditions.json`, thunk table. |
| **[E]** | Unreal Engine 4.25 source in `/opt/fortnite/UnrealEngine`. Corroboration for the original; authoritative only for how the port's own server engine behaves. |
| **[L]** | Run log of the reconstructed server. |
| **[I]** | Inference. Not verified. |

Capture directories (raw decompile `.c`, disassembly `.asm`, callers `.json`), under `/opt/fortnite/local-model-package/captures/`:

| Key | Directory | Contents |
|---|---|---|
| c0 | `c15233ce137b41abb780fbbc03aa8ee3`, `65a6942806aa407e97de21780caaa9bb`, `a8d37eec0bc44249b64a618f728fe705`, `a4d82fe27a1649e1bd8f43e07a5d80e8` | `0x141a18680`, `0x14232ff70`, `0x142314fe0` and `0x14269e270`, `0x1423150b0` |
| c1 | `2f45a58d05894c02b550f8834796803e` | 58 functions: OnReps, RPC thunks, pawn and weapon helpers |
| c2 | `852c8c5c6cfe492884ca0d6c8ccb4fb6` | 25 functions: RPC implementations, holster, WeaponData OnRep, anim layer link |
| c3 | `8ee6eebbcbf74fce89ef0bc2abc8f1f3` | engine and pawn `OnRep_AttachmentReplication`, weapon data init |
| c4 | `563c71eb2f0a44018cad9ec5b25abbec` | 34 functions: every function containing a `+0xc48` or `+0xc58` virtual call, callers of the overlay and visibility helpers, building-tool overrides |
| n1, n2 | `porting/generated/agents/remote-pickaxe-pose/recovery/notify/captures`, `captures2` | attachment notify and its helpers (earlier batch) |

Capture inputs are kept in `porting/generated/agents/remote-pickaxe-pose/lifecycle/batch{1,2,3,4}.json`.

Caller searches used two methods **[B]**: direct `call`/`jmp` references (`porting/tools/callers.py`), and a byte scan
of `.text` for indirect calls through a given vtable offset (`FF /2` or `FF /4` with a 32-bit displacement). The
second finds calls through that offset on *any* class, so each hit was read to see whether the object is a pawn.
Neither method finds calls made through a register loaded elsewhere or through delegates.

Role byte values at actor `+0xf0` **[R]**: 1 SimulatedProxy, 2 AutonomousProxy, 3 Authority.

## 1. Virtual dispatch, resolved from the concrete vtables

All slots below were read directly from the vtable the class constructor installs **[B]**.

Pawn, `AFortPlayerPawnAthena`, vtable `0x145e2cfd0` (constructor `0x141a08ef0`):

| Slot | Target | Identity |
|---|---|---|
| `+0x290` | `0x142339970` | `OnRep_AttachmentReplication`, pawn override |
| `+0x2f8` | `0x14234afa0` | `SetActorHiddenInGame`, pawn override (also drives weapon visibility) |
| `+0x5e8` | `0x143f72550` | `AActor::ForceNetUpdate` (string `NET_AActor_FlushNetDormancy`, then net driver `+0x330`) |
| `+0x6b0` | `0x144430060` | `IsLocallyControlled` **[I]** by use |
| `+0x8e0` | `0x14230e590` | `ClientInternalEquipWeapon_Implementation` |
| `+0x8e8` | `0x1406801d0` | `ServerInternalEquipWeapon_Validate`, returns true |
| `+0x8f0` | `0x14230e590` | `ServerInternalEquipWeapon_Implementation`, **same body as the client one** |
| `+0xab0` | `0x14233f190` | secondary input release (owner only) |
| `+0xab8` | `0x142339c80` | `OnRep_CurrentWeapon` body |
| `+0xb68` | `0x14233f130` | primary input release (owner only) |
| `+0xb78` | `0x1423042a0` | "run weapon equip here": false only for a SimulatedProxy for which `0x142331940` is false |
| `+0xc48` | `0x14234f2a0` | attachment notify, `AFortPlayerPawn` level, calls base `0x14234f080` |
| `+0xc58` | `0x141a18680` | equip, Athena override, wraps `0x14232ff70` |
| `+0xc60` | `0x141a24a80` | `SetHolsterWeapon`, Athena override, calls `0x14234cf20` |
| `+0xc68` | `0x142349bb0` | `SafeHolsterWeapon` |
| `+0x1120` | `0x142319fb0` | `CharacterCharmActors[Index]` |

Earlier reports resolved `+0xc58` from the `AFortPlayerPawn` vtable `0x146116ac8` (`0x14232ff70`). For the Athena pawn
the entry is the override; `+0xab8`, `+0x5e8` and `+0xc48` are the same in both tables.

Weapon, `AFortWeaponPickaxeAthena`, vtable `0x145e4a0f0` (constructor `0x141a69460`, which calls the `AFortWeapon`
constructor `0x14268d9b0`). Every slot in this table equals the `AFortWeapon` base vtable `0x14620a008`; the pickaxe
class overrides none of them.

| Slot | Target | Identity |
|---|---|---|
| `+0x290` | `0x143f7d690` | `AActor::OnRep_AttachmentReplication` (not overridden) |
| `+0x440` | `0x143f84e10` | `AActor::SetOwner` |
| `+0x660` | `0x142695100` | `ClientGivenTo_Implementation` |
| `+0x760` | `0x1426a1760` | given-to-pawn |
| `+0x778` | `0x1426aafb0` | `OnUnEquip` (385 lines, `recovery/weapon-unequip-base.c`) |
| `+0x780` | `0x1426a8510` | `OnEquip(PreviousWeapon)` (762 lines, `recovery/weapon-equip-base.c`) |
| `+0x798` | `0x1426b4ce0` | predicate used for the equip tag branch |
| `+0x7b0` | `0x1426a4940` | stores an item value at weapon `+0x8d0` |
| `+0x7b8` | `0x142690250` | attach to pawn mesh |
| `+0x7c0` | `0x1426ad5b0` | post-attach |
| `+0x7c8` | `0x142697130` | detach |
| `+0x7f8` | `0x14269d6b0` | returns int at weapon `+0x8d4` |
| `+0x8b8` | `0x1426b3730` | set targeting state |
| `+0x958` | `0x1426aad60` | `OnRep_ReplicatedWeaponData` body |
| `+0xa60` | `0x1426b1c00` | tick and mesh update enable |

Other weapon classes **[B]**. `AFortWeaponRanged` (vtable `0x146214010`) overrides `+0x778` (`0x1426d67e0`),
`+0x780` (`0x1426d3660`) and `+0x8b8` (`0x1426dfd80`); **not recovered**, so the pickaxe sequence must not be assumed
for ranged weapons.

`AFortWeap_BuildingTool` (vtable `0x14620fa50`) overrides five slots, read in c4:

| Slot | Target | Difference from the pickaxe |
|---|---|---|
| `+0x7b8` | `0x1426900d0` | The socket passed by the pawn is **ignored**: the default socket is the weapon's own `BlueprintPaperSocket` (`+0xb90`), still replaced by `WeaponHandSocketNameOverride` when set. Same `AttachToComponent` with the same rules object, same `+0x7c0`. Then, if `PencilMeshComponent` (`+0xb88`) exists, that component is attached to the pawn mesh at `BlueprintPencilSocket` (`+0xb98`) with the zero rules object `0x14832ba34` and its relative transform is reset (`0x144122ff0`). |
| `+0x7c8` | `0x1426970a0` | Detaches `PencilMeshComponent` with the KeepRelative rules first, then does what the base detach does. |
| `+0x778` | `0x1426aae00` | If `InstigatorController` (`+0xba0`) is local or the weapon is authority: pawn call `0x14234b570(pawn, 2)` and weapon `+0xaa8(null)`, local-controller build-state calls; then base `OnUnEquip` `0x1426aafb0`; clears `InstigatorController`. |
| `+0x780` | `0x1426a8130` | Calls `0x1426a8290` (**not captured**; not the base `OnEquip` address), then under the same local-or-authority gate `0x14234b570(pawn, 0)` and weapon `+0xaa8(…)`. |
| `+0x798` | `0x1426b4bc0` | Same predicate with an extra test on the previous weapon. |

So a switch to the building tool goes through the same pawn body and the same notify, with a different hand socket
source and a second attached component. `0x14234b570` and `0x1426a8290` were not captured.

Asset ancestry (earlier batch, Cue4Inspect, `recovery/notify/REPORT.md`): `PlayerPawn_Athena_C` →
`PlayerPawn_Athena_Generic_C` → `PlayerPawn_Athena_Generic_Parent_C` → `FortPlayerPawnAthena`;
`B_Athena_Pickaxe_Generic_C` → `FortWeaponPickaxeAthena`. Both outer pawn Blueprints implement `OnWeaponEquipped`;
the pickaxe Blueprint implements `OnWeaponAttached`, `OnWeaponDetached`, `OnWeaponVisibilityChanged`.

## 2. State and who receives it

Fields **[R]**, conditions from the original lifetime registrations **[B]** (`porting/replication_conditions.json`,
registration functions `0x14231f2e0` AFortPawn, `0x14269cf80` AFortWeapon, `0x143f767d0` AActor).

| Field | Offset | Replication | Owner client | Other clients |
|---|---|---|---|---|
| `AFortPawn.CurrentWeapon` | `0x588` | `COND_SkipOwner`, notify `OnRep_CurrentWeapon` | never receives it; sets it locally | receive it |
| `AFortPawn.bWeaponHolstered` | `0x539` mask `0x10` | `COND_SkipOwner`, notify `OnRep_bWeaponHolstered` | sets it locally | receive it |
| `AFortPawn.PreviousWeapon` | `0x590` | not replicated | local | local |
| `AFortPawn.CurrentlyAttachedWeapon` | `0x6d8` | not replicated | local | local |
| `AFortPawn.CurrentWeaponList` | `0x598` | not in the AFortPawn registration | local | local |
| `AFortPawn.CurrentWeaponAnimLayerOverlayClass` | `0xa00` | not replicated | local | local |
| `AFortPawn.WeaponHandSocketName`, `LeftHandWeaponHandSocketName` | `0x5b8`, `0x5c0` | not replicated | class default | class default |
| `AFortWeapon.WeaponData` | `0x350` | `COND_InitialOnly` | initial bunch only | initial bunch only |
| `AFortWeapon.ItemEntryGuid`, `WeaponLevel` | `0x8c0`, – | `COND_InitialOnly` | initial bunch only | initial bunch only |
| `AFortWeapon.bIsEquippingWeapon` | `0x288` | `COND_SkipOwner` | local | receive it |
| `AFortWeapon.EquippedWeaponDestroyWrapperRepCounter` | – | `COND_OwnerOnly` | receives it | never |
| `AFortWeapon.*AbilitySpecHandle` (4) | – | `COND_OwnerOnly` | receive | never |
| `AFortWeapon.WeaponPawnAnimLayerOverlayClass` | `0xac0` | not replicated | class default **[I]** | class default **[I]** |
| `AFortWeapon.WeaponHandSocketNameOverride`, `LeftHand…Override` | `0x9a8`, `0x9b0` | not replicated | class default **[I]** | class default **[I]** |
| `AActor.Owner`, `AActor.Instigator` (weapon) | `0xe0`, `0x118` | `COND_None` | receive | receive |
| `AActor.bHidden` (weapon and pawn) | `0x58` mask `0x01` | `COND_None` | receive | receive |
| `AActor.AttachmentReplication` (weapon) | `0xa0`… | `COND_Custom`, `REPNOTIFY_Always` | when active | when active |

The weapon's pawn is read through `Instigator` (`0x141125940` returns actor `+0x118`) in every weapon helper **[B]**.
`Owner` is checked separately by the equip body (section 3.2 step 1).

Weapon actor constructor defaults, `AFortWeapon` constructor `0x14268d9b0` **[B]**:
`bReplicates = true` (`or byte [this+0x5b],1` at `0x14268dded`); `SetReplicatingMovement(false)` (call to
`0x143f85010` with `edx = 0` at `0x14268ddfd`, which clears `+0x58` mask `0x20`); `NetUpdateFrequency = 1.0`
(`0x14268e07b`). The constructor sets none of `bOnlyRelevantToOwner`, `bAlwaysRelevant`, `bNetUseOwnerRelevancy`.
Blueprint class defaults of `B_Athena_Pickaxe_Generic_C` for these properties were **not** read.

"Class default" for the socket names, the overlay class and the socket overrides is an inference: no constant store to
them was looked for outside the routines below, and the Blueprint defaults were not dumped.

## 3. Server and owner: the equip call chain

### 3.1 Entry points

`AFortPawn::EquipWeaponDefinition` `0x1423150b0` **[B]** (c0). Authority only: returns immediately unless
`Role == 3`.

1. Search `CurrentWeaponList` for a weapon whose `ItemEntryGuid` equals the requested guid and that is not being
   destroyed. If found: call `0x142314fe0(pawn, weapon)` and return. No new actor is spawned.
2. Otherwise resolve the weapon class from the item definition (`0x142672c10`), spawn it deferred with the pawn as
   owner and instigator (`0x1442e6580`), set `WeaponData` and `ItemEntryGuid` (`0x1426b34e0`), for the harvesting-tool
   item type copy pickaxe data from the controller's harvesting tool definition, finish spawning (`0x144211f60`),
   append to `CurrentWeaponList`.
3. Call weapon `+0x760` (`0x1426a1760`): `SetOwner(pawn)`, set `Instigator`, and on authority send the
   `ClientGivenTo` RPC (`0x142b43240`). `ClientGivenTo_Implementation` (`0x142695100`) on the owner does only
   `SetOwner` and set `Instigator`.
4. Call `0x142314fe0(pawn, weapon)`.

Direct callers of `0x1423150b0` **[B]**: `0x141692dd0`, `0x1418b0e00`, `0x14233f230`, `0x1426810a0` and the reflected
thunk `0x142a1b6d0`. Not traced.

Equip wrapper `0x142314fe0` **[B]** (c0), used on every role:

1. If authority and the weapon is non-null: look up the inventory item for `ItemEntryGuid` and pass it to weapon
   `+0x7b0`.
2. `bool ok = pawn->vtable[+0xc58](weapon)`.
3. If `ok`: on authority, send `ClientInternalEquipWeapon(CurrentWeapon)` (`0x142a16c90`) **only if the pawn is not
   locally controlled**; on any other role send `ServerInternalEquipWeapon(CurrentWeapon)` (`0x142a19220`).

Both RPC implementations are `0x14230e590`, which only calls `pawn->vtable[+0xc58](weapon)` **[B]** (c2). Neither
implementation sends anything back. Therefore:

- Server-initiated equip: server runs the equip body, then the owner runs the same body from `ClientInternalEquipWeapon`.
- If a client role ever runs the wrapper: it runs the equip body locally, then the server runs it from
  `ServerInternalEquipWeapon`, and the server does not echo `ClientInternalEquipWeapon`.

All direct callers of the wrapper `0x142314fe0` **[B]** (c4): `EquipWeaponDefinition` (twice, authority only);
`0x14196b640`, which is `AFortGameModeAthena::StartAircraftPhase` by its log strings (game mode, so authority);
and `0x142314d40`, which equips `CurrentWeaponList[0]` and has no direct caller. **No direct caller that runs on a
client role was found.** The two RPC senders `0x142a16c90` and `0x142a19220` are called only from the wrapper and
from the unequip wrapper `0x14232a0c0` (section 3.4). The non-authority branch of the wrapper (local equip, then
`ServerInternalEquipWeapon`) therefore exists in the code, but the client path that would reach it was not found;
it may be reached indirectly or not at all in this build.

### 3.2 Equip body: Athena override `0x141a18680`, base `0x14232ff70`

Athena override `0x141a18680` **[B]** (c0):

1. Return false if `bIsDisconnectedPawn` (pawn `+0xb60`).
2. If the weapon is non-null, global `0x1478c2938 == 1`, the pawn is authority and `0x142331940(pawn)` is true:
   require `0x141ef9880(WeaponData)` (a two-tag query on the weapon data, tag singleton offsets `0xb90`, `0xb98`);
   otherwise return false. `0x142331940` reads `VehicleStateLocal` (pawn `+0x1578`) **[R]** and a byte at `+0x1584`
   and asks the vehicle object a per-seat question (`+0x560`); which question is unresolved.
3. `result = 0x14232ff70(pawn, weapon)`.
4. AutonomousProxy only: zero `LastQuickBarSwitchRequestTime` (`+0x2688`); clear or set a 0.5 s timer on
   `0x141a0e2b0` (quickbar focus check; that callback logs `CheckQuickbarFocusedOnCurrentWeapon`).
5. A trajectory-indicator branch gated by a function that returns false in this image (`Stub_ReturnFalse`).
6. Return `result`.

Base `0x14232ff70` **[B]** (c0). The disassembly was used where the decompile is wrong (noted).

1. **Owner gate.** If the weapon is non-null and `weapon->Owner` is non-null and not this pawn: log
   `EquipWeapon - Weapon %s is owned by %s but %s attempted to being equip it.` and **return false**. The decompile
   shows fall-through; the disassembly (`0x14232ffc7 jc 0x1423300fa`, and the log path ending at
   `0x1423300fa xor al,al`) shows the return.
2. AutonomousProxy only: two list operations on `CurrentWeaponList` with a null entry and with the weapon
   (`0x140726fc0`, `0x14085a990`); remove-null and add-unique is the likely reading **[I]**.
3. If the weapon equals `CurrentWeapon`: **return false** (`0x14233014b cmp`, `0x142330152 jz 0x1423300fa`). No RPC
   follows, because the wrapper sends only on true.
4. Gameplay tag bookkeeping on the pawn with the tag at singleton offset `0x6c0` (pawn `+0xcd8`, `+0xce8`, `+0xcf0`),
   branch chosen by weapon `+0x798(CurrentWeapon)`. The tag's name was not resolved.
5. If there is an old `CurrentWeapon`: call old `+0x778` (`OnUnEquip`); `CurrentWeapon = null`; **if the new weapon is
   null, call pawn `+0xc48`** (attachment notify); broadcast `OnWeaponUnEquippedDelegate` (`+0x9c0`).
6. If the pawn's ability system component is a Fort ASC: send a gameplay event (ASC `+0x620`, tag at singleton offset
   `0x160`) with instigator and target = pawn, optional objects = new and old weapon.
7. If the new weapon is non-null:
   a. controller-dependent clear of `bSecondaryInputHeld` (pawn `+0x522` mask `0x20`);
   b. `CurrentWeapon = weapon`;
   c. pawn `+0x5e8` (`ForceNetUpdate` on the pawn);
   d. if pawn `+0xb78` is true: on non-authority with a null weapon instigator, set the weapon's instigator to the
      pawn; then call weapon `+0x780(old)` (`OnEquip`);
   e. pawn `+0xc48` (attachment notify, section 4);
   f. if a controller flag (`+0x3c8` mask `2`) is set, call weapon `SetActorHiddenInGame` with a controller byte
      (`+0x62a`).
8. Broadcast a global native delegate at `0x1478de548` with (pawn, CurrentWeapon, old).
9. Return true (`0x1423304dc mov al,1`).

`PreviousWeapon` is not written here. It is written only by the attachment notify (section 4 step 1).

### 3.3 Weapon `OnEquip` `0x1426a8510` and `OnUnEquip` `0x1426aafb0` (partial)

Read for role gates and attachment-relevant effects only **[B]** (`recovery/weapon-equip-base.c`,
`recovery/weapon-unequip-base.c`). Not a full recovery.

`OnEquip(Previous)`: resets weapon-local counters; calls weapon `+0x5e8` (`ForceNetUpdate` on the weapon, line 69);
with a previous weapon, sets a timer on `0x1426afaa0`; grants gameplay tags and abilities from `WeaponData`
(authority and owner branches); authority-only controller call `0x1423bebe0`; if weapon bit `+0x900` mask `0x08` is
clear: set it, clear targeting on both weapons, `+0xa60(true)`, and, with an instigator pawn for which `0x142331940`
is false, rebind montage delegates (`0x142694ec0`, `0x1426a27c0`) and call `0x1426abce0(weapon, previous)`;
authority-only branch that updates a player-state object and forces its net update; broadcast of a weapon delegate at
`+0x2a0`; a deferred callback on `0x1426ad0e0`.

`OnUnEquip`: clears a flag at `+0x9c4`, broadcasts a weapon delegate at `+0x2f0`, calls weapon `+0x758`, stops
ability or reload state, removes granted tags, calls the Blueprint event thunk `0x142b43850`; if bit `+0x900` mask
`0x08` is set: clear it, `+0x690` (stop persistent effects), `+0xa60(false)`, unbind montage delegates; an
authority-only player-state update; broadcast of a weapon delegate at `+0x290`.

`OnUnEquip` does **not** detach the actor or change its visibility. Those happen only in the attachment notify.

The bit at weapon `+0x900` mask `0x08` has no reflected name ("equipped and initialised" by use **[I]**). It is set by
both `OnEquip` and by `0x1426a9eb0` in the attachment notify, whichever runs first.

### 3.4 Unequip entry points

Found by the caller searches **[B]** (c4):

- `0x14232a0c0` (direct caller `0x141a16670`; 1,053 bytes, only the head was read): calls
  `pawn->vtable[+0xc58](null)`; on true, authority with a non-local pawn sends `ClientInternalEquipWeapon(CurrentWeapon)`
  (now null), any other role sends `ServerInternalEquipWeapon`. What the function does afterwards and what it is
  were not established.
- `0x142358960(pawn, bNotify)` (direct callers `0x1419571b0`, `0x1423b0340`, `0x14205cbf0`, not captured): if
  `CurrentWeapon` is set, call its `OnUnEquip`, set `CurrentWeapon = null`, run the attachment notify **only if
  `bNotify`**, broadcast `OnWeaponUnEquippedDelegate`. This bypasses the equip body: no tag bookkeeping, no ability
  event, no RPC.

## 4. Attachment notify: pawn `+0xc48`

Runs on every role. Complete list of callers found by both search methods **[B]**: the equip body (steps 5 and 7e
above), `OnRep_CurrentWeapon` (`0x142339c80`) and `0x142358960` (section 3.4). The other seven `+0xc48` call sites
in the image take different arguments and belong to other classes (game mode, game state and others; c4).
The base `0x14234f080` has one more direct caller, `0x1422ed330` (not captured).

Derived `0x14234f2a0` (n1): if `bShowingOverdriveEffect`, call `0x142358250` first (weapon material effect, not
recovered); call base; then, if `CurrentlyAttachedWeapon` and `CharacterCharmActors[0]` exist, re-attach the charm to
the weapon (`0x1419bfbf0`).

Base `0x14234f080` (n1, `recovery/pawn-notify.asm`):

1. `PreviousWeapon = CurrentlyAttachedWeapon`.
2. If `CurrentlyAttachedWeapon` is non-null and equals `CurrentWeapon`: **return**. Nothing is re-attached, no event
   fires, no delegate is broadcast.
3. If there was an attached weapon (now different):
   a. targeting off on the old weapon (`+0x8b8(false)`): always for a SimulatedProxy; for other roles only when
      `CurrentWeapon` is an `AFortWeaponRanged` and either `0x1426e7770(CurrentWeapon)` is true or the pawn is an
      `AFortPlayerPawnAthena`;
   b. `0x1426a9f90(old)`: clear bit `+0x900` mask `0x08`, zero `PersistentFXStartTime`, `+0x690`, `+0xa60(false)`,
      unbind montage delegates;
   c. `0x1426b3b20(old, visible = false, force = true)` (section 5);
   d. old `+0x7c8` = `0x142697130`: a reload-related cleanup when the weapon data has a positive value at `+0x128`,
      then `AActor::DetachFromActor` (`0x143f6cf60`: `RootComponent->vtable[+0x400](rules)`) with the rules object at
      `0x147cdd8e0`, then the Blueprint event thunk `0x142b44210` (`OnWeaponDetached` **[I]** by position);
   e. `CurrentlyAttachedWeapon = null`.
4. If `CurrentWeapon` is non-null:
   a. `0x14234b3f0(pawn, &CurrentWeaponAnimLayerOverlayClass, CurrentWeapon->WeaponPawnAnimLayerOverlayClass)`.
      Requires `Mesh` (pawn `+0x278`); if null it only logs
      `AFortPlayerPawn::SetAnimLayerOverlayClassInternal without valid SkelMeshComp`. If the stored class is a
      `UAnimInstance` subclass it is unlinked from the mesh (`0x1441286e0`); the new class is stored; if it is a
      `UAnimInstance` subclass it is linked (`0x14411b8c0`). Both helpers forward to the mesh's anim instance at
      mesh `+0x630` and do nothing when that is null. `LinkAnimClassLayers` / `UnlinkAnimClassLayers` is the reading
      **[I]**; the class test against `AnimInstance` is **[B]** (`0x1446b91c0`).
   b. weapon `+0x7b8(Mesh, WeaponHandSocketName, LeftHandWeaponHandSocketName)` = `0x142690250` (n1): replace each
      socket name with the weapon's `…Override` when that is not `None`; call `0x143f69390(weapon, Mesh, rules at
      0x147cdd8dc, socket)`; then weapon `+0x7c0(Mesh, socket, leftSocket)` = `0x1426ad5b0` (n2): for roles below
      authority, point the weapon's nine audio components (`+0x748`…`+0x788`) at the mesh and socket; store a value in
      `CustomTimeDilation`; set the pawn's `bPawnLODDirty`; fire the Blueprint event thunk `0x142b441d0`
      (`OnWeaponAttached` **[I]**); tail-call weapon `+0x950` (`0x14268fd70`, cosmetic customisation load, not
      recovered).
   c. `CurrentlyAttachedWeapon = CurrentWeapon`.
   d. `0x1426b3b20(CurrentWeapon, visible = !pawn.bHidden && !bWeaponHolstered, force = true)`.
   e. `0x1426a9eb0(CurrentWeapon, PreviousWeapon)`: if bit `+0x900` mask `0x08` is clear, set it; with a previous
      weapon clear targeting on both; `+0xa60(true)`; with an instigator pawn (subject to a vehicle-style test whose
      result the decompile does not show being used) rebind montage delegates and call `0x1426abce0` (222 lines,
      reads the pawn mesh's anim instance; not recovered).
   f. Blueprint event `OnWeaponEquipped(CurrentWeapon, PreviousWeapon)` on the pawn (`0x142a19190`).
   g. Broadcast `OnWeaponEquippedDelegate` (`+0x9b0`).
   h. Set the pawn bit at `+0x521` mask `0x08` (no reflected name; read by weapon `+0x798`).
   i. Non-authority only: weapon `+0x7f8`, build an item descriptor (`0x14269e270`: guid, definition name, two
      bytes), call a function that is empty in this image.
5. Always (also when `CurrentWeapon` is null): broadcast `OnWeaponAttachmentChangedDelegate` (`+0x9d0`).

The weapon overlay is also re-applied outside the notify **[B]** (all direct callers of `0x14234b3f0`, c4):

- `0x14234b2e0`, `AFortPlayerPawn::SetAnimInstanceClassInternal` by its log string: after changing the mesh's anim
  class it calls `0x14234b3f0(pawn, &CurrentWeaponAnimLayerOverlayClass, CurrentWeapon->WeaponPawnAnimLayerOverlayClass)`
  when `CurrentWeapon` is non-null. Callers: `0x14233a160` (the vehicle state OnRep, by its
  `AFortPlayerPawn::OnRep_InVehicle` strings) and `0x1423393a0` (not captured).
- `0x14235a560`, the post-load part of `AFortPlayerPawn::UpdateCharacterCustomization` by its strings: same call
  when `CurrentWeapon` is non-null. Caller `0x142309ce0` (not captured).
- `0x142332d20`, `0x1423589c0` and three sites in `0x14233a160` use the same helper for other overlay slots
  (swimming at pawn `+0x25c8`, a vehicle slot at `+0x25c0`), not the weapon slot.

These re-apply the **overlay** from `CurrentWeapon` (not from `CurrentlyAttachedWeapon`). They do not re-run the
attach, the visibility call or the Blueprint events.

### Attachment and detachment rules

`0x143f69390` is `AActor::AttachToComponent(Parent, Rules, Socket)` with the scene-component attach inlined: it reads
the actor's `RootComponent` (`+0x130`), carries the engine's `AttachToSelfWarning`, `AttachToSelfRootWarning`,
`AttachCycleWarning` and `NoStaticToDynamicWarning` messages, and reads the rules as four bytes
(`[0]` location, `[1]` rotation, `[2]` scale, `[3]` weld) **[B]** (n2).

The rule objects are constant-initialised data, not written at run time **[B]**: the same bytes are in the on-disk
executable's `.data` (`/mnt/vibrato/Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe`) and in
the runtime dump, and a scan of `.text` for RIP-relative stores into `0x147cdd8cc`–`0x147cdd8e7` found none.

| Address | Bytes | Decoded | Used by |
|---|---|---|---|
| `0x147cdd8d4` | `01 01 01 00` | attach: KeepWorld ×3, no weld | – |
| `0x147cdd8d8` | `02 02 01 00` | attach: SnapToTarget, SnapToTarget, KeepWorld (not including scale) | – |
| `0x147cdd8dc` | `02 02 02 00` | attach: **SnapToTarget ×3 (including scale)**, no weld | pickaxe attach `0x142690250`, building tool attach `0x1426900d0`, charm attach `0x1419bfbf0` |
| `0x147cdd8e0` | `00 00 00 01` | detach: **KeepRelative ×3**, call-modify true | weapon detach `0x142697130` |
| `0x147cdd8e4` | `01 01 01 01` | detach: KeepWorld ×3, call-modify true | `OnRep_AttachmentReplication` detach |

Enum values follow UE 4.25 `EAttachmentRule` / `EDetachmentRule` (0 KeepRelative, 1 KeepWorld, 2 SnapToTarget)
**[E]**. The three attach objects and two detach objects sit next to each other in the order the engine declares
its `FAttachmentTransformRules` and `FDetachmentTransformRules` statics; the bytes before them (`0x147cdd8d0`:
`00 01 00 00`) are not a rules object, and the all-zero KeepRelative attach object is not in this group (the zero
object at `0x14832ba34`, section 7, is the likely one **[I]**).

Result for the pickaxe **[B]**: on every machine that runs the notify, the weapon actor's root component is attached
to the pawn's `Mesh` at `WeaponHandSocketName` (or the weapon's override) with location, rotation and scale all
snapped to the socket. The left-hand socket name is passed on to `+0x7c0` and is not used for the attach.

## 5. Visibility and holster

`0x1426b3b20(weapon, bVisible, bForce)` **[B]** (n1): if `bForce` or the weapon's `bHidden` disagrees: weapon
`SetActorHiddenInGame(!bVisible)` (`+0x2f8`, `AActor` version `0x143f83280`) and the Blueprint event thunk
`0x142b44250(bVisible, false)` (`OnWeaponVisibilityChanged` **[I]**). Then, if the instigator is a player pawn, set the
first charm's visibility (`0x1419f38a0`). Then `0x1426b3990`: store `!bVisible` at weapon `+0xb80` and hide or show
the `AttachedTrajectoryIndicator`.

All direct callers **[B]**: attachment notify (steps 3c, 4d); pawn `SetActorHiddenInGame` override `0x14234afa0`
(`visible = !bNewHidden && !bWeaponHolstered`, not forced); `OnRep_bWeaponHolstered` body `0x14233e380`;
`SetHolsterWeapon` base `0x14234cf20`; the reflected `SetWeaponVisibility` thunk `0x142b48c60`; and six functions
captured but not identified: `0x141a10420` (Athena pawn vtable `+0x330`; forces the current weapon hidden),
`0x14234ae40`, `0x14234e230`, `0x14234e320` (each computes a visibility for `CurrentWeapon`), `0x142704670`,
`0x14270d060` (not captured).

Because the weapon's `bHidden` also replicates (`COND_None`), a remote client can receive weapon visibility from two
sources: the replicated flag on the weapon actor and its own local computation in the notify and holster paths. Which
arrives last on a given client was not measured.

`OnRep_bWeaponHolstered` thunk `0x142a1ece0`, body `0x14233e380` **[B]** (c1, c4): only if `CurrentWeapon` is non-null **and
equals `CurrentlyAttachedWeapon`**: `0x1426b3b20(weapon, !bWeaponHolstered, true)`; locally controlled only: resume or
stop firing state. Then broadcast one of two pawn delegates at `+0xa08` / `+0xa18`. If the holster bit arrives before
the notify has attached the weapon, this OnRep changes nothing; the notify's step 4d reads the bit later.

`SetHolsterWeapon` `0x141a24a80` → `0x14234cf20` **[B]** (c2): a SimulatedProxy call is ignored unless a force
argument is set; maintains a list of holster request ids (`Unholster id: %s, no matching holster call found`);
writes `bWeaponHolstered`; then the same visibility and firing-state handling as the OnRep. `SafeHolsterWeapon`
`0x142349bb0` is non-simulated only and forwards to `+0xc60`.

## 6. What other clients receive: `OnRep_CurrentWeapon`

Thunk `0x142a1ead0` → pawn `+0xab8` = `0x142339c80` **[B]** (c1):

```
pawn->vtable[+0x5e8]()    // ForceNetUpdate; on a client only the trailing net-driver call can run
pawn->vtable[+0xc48]()    // attachment notify, section 4
```

It does **not** call the equip body, weapon `OnEquip`, weapon `OnUnEquip`, `OnWeaponUnEquippedDelegate`, the ability
event, or the gameplay tag bookkeeping. On a remote client the weapon's `+0x900` mask `0x08` bit, tick enable, montage
delegates, anim layer, attachment, visibility and the Blueprint `OnWeaponEquipped` / `OnWeaponAttached` events all
come from the notify alone.

The owner never runs this OnRep (the property skips the owner). The owner's notify runs from the equip body.

## 7. Second attachment source: engine attachment replication

Weapon `OnRep_AttachmentReplication` is the engine's, `0x143f7d690` **[B]** (c3):

- `AttachmentReplication.AttachParent` (actor `+0xa0`) null: `RootComponent->DetachFromComponent` with the KeepWorld
  rules at `0x147cdd8e4`; then, only if `bReplicateMovement`, `OnRep_ReplicatedMovement`.
- Non-null, with `AttachComponent` or the parent's root: set the root's relative location, rotation and scale from
  the replicated struct (`0x144123220`, `0x144123460`, `0x144123690`); if the root is already attached to that
  component at that socket and listed among its children, only refresh; otherwise attach (`0x14410a300`) with the
  rules object at `0x14832ba34`. That object is zero in the runtime dump and lies in the zero-initialised part of
  `.data`; no store to it was found. All-zero decodes as KeepRelative ×3 with no weld, which is the engine's
  `KeepRelativeTransform` **[E]**; an uninitialised object would decode the same, so the effect is the same either way.

The pawn overrides its own `OnRep_AttachmentReplication` (`0x142339970`); that is about the pawn's attachment (to
vehicles), not the weapon's.

When the server sends the property is engine code that was not located in the image. UE 4.25 **[E]**:
`AActor::PreReplication` activates it only while `RootComponent && !RootComponent->GetIsReplicated()`
(`Actor.cpp:1061`); `GatherCurrentMovement` runs when the actor replicates movement **or its root has an attach
parent** and then fills parent, component, socket and relative transform (`ActorReplication.cpp:335-376`).
The weapon does not replicate movement (section 2). On an engine that behaves like 4.25 this has two effects
**[E]**: the attachment is gathered and sent while the weapon's root is attached on the server; and once the root
is detached, `GatherCurrentMovement` is not entered at all (neither condition holds), so the struct keeps the last
parent and **a detach is never sent**.

Consequences that follow from the recovered code, each marked:

- In the original, the server runs the notify (it runs on every role), so the server's weapon root is attached to the
  pawn mesh **[B]**. Whether the original server's engine then replicated that attachment is **[E]/[I]**, not read
  from a server binary.
- A remote client can therefore attach the weapon twice, from two independent sources with different rules:
  locally with SnapToTarget ×3 (section 4), and from replication with the server's relative transform under
  KeepRelative. After a server SnapToTarget attach the server's relative transform is identity apart from the
  socket, so both give the same placement if the same component and socket are named **[I]**.
- A received `AttachmentReplication` with a null parent would detach the weapon root with KeepWorld on the client
  **[B]**, and the notify's early return (section 4 step 2) would not re-attach it. By the previous paragraph a
  4.25-like server does not send a null parent for a weapon that does not replicate movement **[E]**, so this
  needs a different trigger (movement replication enabled by a Blueprint default, or a parent that fails to
  resolve on the client). Not observed either way.
- A client that starts receiving a weapon whose root is attached on the server gets the attachment in the weapon's
  own data, independent of the pawn's `CurrentWeapon` **[E]**. A weapon that was attached once and later unequipped
  keeps its stale parent on the server side of replication **[E]**; what a late-joining client does with a stale
  attachment for a weapon that is not the pawn's `CurrentWeapon` was not examined.
- Arrival order of the weapon's `AttachmentReplication`, the pawn's `CurrentWeapon` and the weapon's initial bunch
  on a given remote client is **unmeasured**.

## 8. `WeaponData` on clients

`OnRep_ReplicatedWeaponData` thunk `0x142b47b20` → weapon `+0x958` = `0x1426aad60` **[B]** (c2): returns if
`WeaponData` is null; if the object pointed to by the global at `0x14816d820` has a zero first field (the test is `*global == 0`),
calls weapon `+0x9d8` (`0x1426ae490`) directly;
otherwise resolves a soft reference from the weapon data (`0x14266e250`) and either calls `0x1426a6bb0` at once or
starts an async load (`RequestAsyncLoad SingleDelegate`) that calls it later. `0x1426a6bb0` (334 lines) was captured
and not read beyond checking that it does not write the overlay class or socket overrides.

So weapon initialisation from `WeaponData` on a client can complete after the attachment notify has already run.
The notify itself reads only the weapon's overlay class and socket overrides, which were not seen being written from
`WeaponData`. Which other notify-reachable code (`0x1426abce0`, `+0x950`, Blueprint events) depends on finished
weapon-data initialisation is unresolved.

## 9. Sequences per audience

S = server (authority), O = owning client, R = any other client. "notify" is section 4.

### 9.1 Server-initiated equip (spawn, pickup, `EquipWeaponDefinition`)

| # | Where | What | Reaches |
|---|---|---|---|
| 1 | S | `0x1423150b0`: find or spawn weapon, owner and instigator = pawn, `WeaponData`, `ItemEntryGuid` | weapon actor replicates to O and R (relevancy rules not recovered); `WeaponData`, guid in the initial bunch only |
| 2 | S | weapon `+0x760`: `SetOwner`, `Instigator`, `ClientGivenTo` RPC | O (RPC); Owner and Instigator to O and R |
| 3 | S | `0x142314fe0` → `+0xc58` → `0x14232ff70`: old `OnUnEquip`, `CurrentWeapon` store, pawn `ForceNetUpdate`, new `OnEquip` (weapon `ForceNetUpdate`), notify (attach on server) | `CurrentWeapon` to R only; server root attachment to O and R through `AttachmentReplication` **[E]** |
| 4 | S | `ClientInternalEquipWeapon(weapon)` if the pawn is not locally controlled | O only |
| 5 | O | `0x14230e590` → `+0xc58` → same body: owner gate, `CurrentWeapon` store, `OnEquip`, notify | local |
| 6 | R | `OnRep_CurrentWeapon` → notify only | local |

Preconditions on O for step 5 **[B]**: the RPC's weapon reference must resolve; `weapon->Owner` must be null or the
pawn (step 1 of the body), else the equip is refused with a log line and `CurrentWeapon` stays unchanged on the owner.

### 9.2 Owner-initiated switch

What the owning client sends when the player presses a quickbar key was **not recovered** as a call chain.
What is established **[B]**:

- No client-role direct caller of the equip wrapper exists (section 3.1), so a locally predicted equip followed by
  `ServerInternalEquipWeapon` is not shown to happen.
- `ServerEquipLastWeaponOrGadget_Implementation` (`0x14234a320`) and its helper `0x142314d70` go through a
  controller virtual (`+0x1b10`, item guid) rather than through the pawn equip.
- `AFortPlayerController::ServerExecuteInventoryItem` has only a stub body in the client
  (`server_systems/D5-stubbed-rpcs-and-recreation.md`).

The reading consistent with these is: the owner sends an inventory request, the server answers with 9.1, and the
owner's `CurrentWeapon` changes only when `ClientInternalEquipWeapon` arrives **[I]**. Under that reading the
sequence for S, O and R is exactly 9.1 steps 1 (existing weapon found in `CurrentWeaponList`, no spawn) and 3 to 6.

If the wrapper does run on a client role, the alternative is: O runs the full body and notify locally, sends
`ServerInternalEquipWeapon`; S runs the body (`CurrentWeapon` to R, attachment as above) and sends nothing back to O;
R runs `OnRep_CurrentWeapon` → notify. `ServerInternalEquipWeapon_Validate` always returns true in this image.

### 9.3 Switch away and back within one server frame (pickaxe → building tool → pickaxe)

On S the body runs for each switch **[B]**. For R, `CurrentWeapon` is compared against what that connection was last
sent; a value that returns to the previous actor before the pawn is next replicated produces no change and no OnRep
(`REPNOTIFY_OnChanged` is the default; no notify condition is registered for `CurrentWeapon`) **[R]/[E]**. If the
intermediate value is sent, R runs the notify twice and detaches and re-attaches. If S detaches and re-attaches the
pickaxe root in the same frame, the gathered `AttachmentReplication` is unchanged at the next replication **[E]**.
Which of these happened in the run logged at `server-equip-excerpt.log` (building tool equip and pickaxe re-equip in
the same frame for client 2) **[L]** was not measured.

### 9.4 Initial replication and reversed join order

For a client R that starts receiving a pawn that already holds a weapon:

- R gets the pawn's `CurrentWeapon` in the pawn's bunch and the weapon actor on its own channel. If the weapon actor
  is not yet known when the pawn property arrives, the reference is unresolved; UE 4.25 resolves it and calls the
  notify when the actor arrives **[E]**. Not verified in the image.
- The weapon's native `NetUpdateFrequency` is 1.0 **[B]**. The original forces updates at equip time (pawn in the
  body, weapon in `OnEquip`); a client that joins later gets no such forced update from the equip path, so the weapon
  channel's opening time relative to the pawn's is governed by ordinary scheduling **[I]**.
- R's notify needs, at the time it runs: the pawn's `Mesh` and its anim instance (else the overlay class is stored
  but the link is skipped silently), the pawn's socket names, `bWeaponHolstered` and the pawn's `bHidden`, and the
  weapon's overlay class and socket overrides **[B]**.
- The notify does not rerun for the same `CurrentWeapon` (step 2), and it is the only code found that attaches the
  weapon, sets `CurrentlyAttachedWeapon` and fires the equip and attach events (section 4 caller list) **[B]**.
- The overlay alone is re-applied later by `SetAnimInstanceClassInternal` and by the post-load step of
  `UpdateCharacterCustomization` (section 4) **[B]**. So an overlay link that was skipped because the mesh had no
  anim instance yet is repaired when either of those runs afterwards; whether they do run after the notify on a
  given client, and in which order character customisation and `CurrentWeapon` arrive, is **unmeasured**.
- If the mesh's skeletal mesh asset changes after the attach, the attachment to the component and socket name
  persists **[E]**; whether the original re-attaches on a mesh change was not found in the code read.
- The first joiner sees the second joiner's pawn spawn live; the second joiner receives the first joiner's pawn and
  weapon as existing actors. The two directions therefore exercise different orderings of the same OnReps. The
  actual orderings in either direction are **unmeasured**.

### 9.5 Unequip and weapon destruction

- Equip body with a null weapon **[B]**, reached through `0x14232a0c0` (section 3.4): old `OnUnEquip`,
  `CurrentWeapon = null`, notify (detach old with KeepRelative, hide it, clear `CurrentlyAttachedWeapon`, broadcast
  attachment-changed), `OnWeaponUnEquippedDelegate`, ability event; then `ClientInternalEquipWeapon(null)` to a
  non-local owner. R receives `CurrentWeapon = null` and runs the notify.
- `0x142358960(pawn, bNotify)` **[B]**: `OnUnEquip`, `CurrentWeapon = null`, notify only if asked, delegate. R still
  receives `CurrentWeapon = null` when this runs on the server; O receives nothing from this function.
- `OnRep_EquippedWeaponDestroyWrapper` (`0x142b47a80`, owner only) **[B]**: targeting off, release primary and
  secondary input on the pawn, play a sound at the weapon's location, fire a Blueprint event (`0x142b43b20`).
- What the server does when an equipped weapon is destroyed or removed from the inventory (which of the two entry
  points above it uses, which weapon is equipped next, when the counter is bumped, when the actor is destroyed) was
  not traced: the callers of both entry points are uncaptured.

## 10. Server-only or unresolved behaviour

Present in the client image and recovered here: everything in sections 3 to 8 that carries **[B]**, including the
authority branches of `EquipWeaponDefinition`, the equip wrapper and the equip body.

Stubbed, absent or not located in the client image:

| Item | State |
|---|---|
| `AFortPlayerController::ServerExecuteInventoryItem` implementation | Stub in the client (`server_systems/D5-stubbed-rpcs-and-recreation.md`). How the original server maps a quickbar request to an equip is not recoverable from this image. |
| Server handling of destroyed or removed equipped weapons | Entry points found (section 3.4); their callers `0x141a16670`, `0x1419571b0`, `0x1423b0340`, `0x14205cbf0` not captured. |
| Owner-side request for a weapon switch | Call chain not recovered (section 9.2). |
| Weapon actor relevancy on the original server | Native constructor sets no relevancy flags; Blueprint defaults and any replication-graph policy not read. |
| Server-side `PreReplication` / `GatherCurrentMovement` for the weapon | Not located in the image; engine source only. |
| `ServerEquipLastWeaponOrGadget` | Implementation `0x14234a320` present: if `0x142314d70` fails and `bForce`, pawn `+0xbc0`. `0x142314d70` picks a remembered item guid and calls controller `+0x1b10`; that target was not captured. |
| Function called at notify step 4i | Empty in this image; may have a body on other targets. |

Recovered only partially (bodies captured, semantics not established):

`0x1426a8510` and `0x1426aafb0` (weapon equip and unequip: ability, tag, timer and player-state branches);
`0x1426abce0`, `0x1426a27c0` (montage and animation work in the notify); `0x14268fd70` (weapon `+0x950`);
`0x1426a6bb0`, `0x1426ae490` (weapon data initialisation); `0x142358250` (overdrive material effect);
`0x1419d2c30`, `0x1419bfbf0` (charm socket and attach); `0x142331940`; the tag at singleton offset `0x6c0`; the pawn
bit `+0x521` mask `0x08`; the weapon bit `+0x900` mask `0x08`; the ranged overrides and the building tool's
`0x1426a8290` and `0x14234b570` (section 1); `0x14232a0c0` beyond its head; the unidentified visibility callers
(section 5); `0x1422ed330`; the Blueprint bodies of `OnWeaponEquipped`, `OnWeaponAttached`,
`OnWeaponDetached`, `OnWeaponVisibilityChanged` (present in the assets, bytecode not read).

## 11. The current port against this contract (facts, no fix proposed)

From `recovery/notify/REPORT.md` and `remote-pickaxe-pose/REPORT.md`; the port source was not re-read for this
document: `FortAthenaInventoryFlow.cpp:169-187` assigns `PreviousWeapon` and `CurrentWeapon` directly and forces
replication; it does not call the equip body or the attachment notify, and a later tick sends
`ClientInternalEquipWeapon` to the owner.

Measured against sections 3 to 7, these original server-side effects have no counterpart in that description:
weapon `+0x760` and `ClientGivenTo`; old-weapon `OnUnEquip`; new-weapon `OnEquip`; the server-side attach of the
weapon root to the pawn mesh (and with it any `AttachmentReplication`); the server-side overlay link, visibility
call, Blueprint events and delegates; `CurrentlyAttachedWeapon`; and `PreviousWeapon` being written by the notify
rather than by the equip.

None of this establishes the cause of the reported pose difference. Both remote clients run the same local notify;
a missing server attach affects every observer equally unless ordering differs.

## 12. Measurements the contract makes possible

Per observer and per direction, around one explicit pickaxe → building tool → pickaxe switch and around each join:

1. On the remote client: time of `OnRep_CurrentWeapon`; at that moment the pawn's `Mesh` and anim instance
   (mesh `+0x630`), `bWeaponHolstered`, pawn `bHidden`; after it `CurrentlyAttachedWeapon`,
   `CurrentWeaponAnimLayerOverlayClass`, the weapon root's attach parent and socket.
2. Whether and when the weapon receives `AttachmentReplication`, with which parent, and whether a null-parent
   notify ever follows a local attach.
3. Whether the weapon actor channel opens before or after the pawn's `CurrentWeapon` arrives, and when
   `OnRep_ReplicatedWeaponData` finishes.
4. On the owner: whether `ClientInternalEquipWeapon` passes the owner gate (no `EquipWeapon - Weapon … is owned by`
   line in the client log).
5. On the server: weapon root attach parent and the weapon's `Owner` and `Instigator` at each transition.
