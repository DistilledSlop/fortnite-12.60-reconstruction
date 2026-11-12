# R4: readiness, building and interaction RPCs, read from the client

Date: 2026-11-12. Same conventions and confidence labels (**read**,
**inferred**, **not read**) as `R1-warmup-and-aircraft-code.md`. This card
covers the request handlers on the G6 to G8 path that earlier cards list as
real code. One of them is not.

## 0. Corrections to earlier cards

1. **`ServerCreateBuildingActor` has no building logic in the client.**
   `G6_MAPPING_APPROACH.md` and `D6-combat-and-elimination.md` list it as
   real (`0x1423aad00` then `0x14236d1a0`), and the sweep in
   `D5-stubbed-rpcs-and-recreation.md` gives it the verdict `mixed`. Section
   2 shows why that is wrong. It belongs with the re-created building RPCs.
2. `0x14236d1a0` is not part of that RPC. It is a separate helper for the
   extra preview piece, called from `0x1426b0790` only. It is still the best
   model in the binary for how the server spawns a building (section 2.2).

## 1. Readiness: who sets `bReadyToStartMatch` on the server (read)

Card R1 section 4.1 shows that the warmup counter only counts controllers
with `bReadyToStartMatch` (controller byte `+0x658`, mask 0x20).

- `ServerReadyToStartMatch` exec thunk `0x142a4c360`: `_Validate` is
  controller slot `+0x1340`, `_Implementation` is slot `+0x1348`.
- `_Implementation` (`0x1423abda0`) is a single tail call to controller
  slot `+0x1590`, which is `OnReadyToStartMatch`.
- `OnReadyToStartMatch` (`0x14239ccf0`, 194 lines):
  1. return if the bit is already set;
  2. log `OnReadyToStartMatch: Marking player %s ready to start`;
  3. **set `bReadyToStartMatch`**;
  4. create the `StatManager` object if missing and call its slot `+0x260`
     with the controller;
  5. controller slot `+0x1c98`;
  6. for each object in a list built by `0x1423644c0(controller)` call its
     slot `+0x408` (inferred: the quick bars and components created at
     ready time; `G6_MAPPING_APPROACH.md` says the ClientQuickBars are
     created here);
  7. on the authority only: copy `CachedPersistentGameplayStats` into the
     stat manager when the `MainMcpProfile` has them, then team and
     PlayerState setup that was not read past this point.
- The client's 0.5 s timer callback (`0x1423bc750`) is the other caller:
  when the readiness check (slot `+0x14e8`) passes it sends
  `ServerReadyToStartMatch` (`0x142a3f150`, when not the authority) and then
  calls `OnReadyToStartMatch` locally.

**For the server:** the generated `ServerReadyToStartMatch_Implementation`
must reach `OnReadyToStartMatch`, and `OnReadyToStartMatch` must be ported
at least through step 3. If the RPC arrives and the bit stays clear, the
warmup early countdown never starts and the forced bus exit kicks the player
(`R1` section 7.2). Check this bit first in any G7 run.

## 2. `ServerCreateBuildingActor` (read)

### 2.1 What is in the binary

- Exec thunk `0x142a4aab0`: reads one struct argument (0x38 bytes,
  `FCreateBuildingActorData`), calls controller slot `+0x1178`
  (`_Validate`) and then slot `+0x1180` (`_Implementation`).
- In the controller vtable the two neighbouring entries are the shared
  "return true" stub (`0x1406801d0`) and `0x1423aad00`, in that order,
  between entries that follow the usual stubbed-RPC pattern. So `_Validate`
  is the trivial stub and **`0x1423aad00` is `_Implementation`**.
- `0x1423aad00` is 23 instructions. It gets the world's GameState, checks it
  is the Athena class, and checks that the first int of the argument (the
  building class handle) is `>= 0` and `< GameState.AllPlayerBuildableClasses.Num()`
  (count at GameState `+0xcf0`). Then it returns. The assembly sets a
  register to 1 or 0 on the two paths and discards it. Nothing is spawned.

Reading: the guard survived and the body after it was compiled out, the
same way as the other server-only bodies. The class handle being an index
into `AllPlayerBuildableClasses` is real information; the rest of the
argument struct (location, rotation, mirrored flag) has to be taken from the
reflected struct layout.

### 2.2 The model to copy: the extra-piece helper, `0x14236d1a0` (read)

`(PC, World?, BuildingClass, Location*, Rotation*, bMirrored, bFlag2,
uint8 Mode)`. It runs only when the runtime option byte `+0x9d8` is set and
`Mode != 0`, so it is not the normal build path, but it shows the server
steps for one building actor:

1. The class must derive from the building actor base class
   (`0x142752f80`).
2. Get the structural grid object from the world (`0x141c77490`), snap the
   location to the grid (`0x141c78270`), adjust for `Mode` 1 or 2, and
   compute the final transform (`0x141c7dbd0`).
3. Ask the controller whether it may build there: controller slot `+0x1a20`
   `(BuildingClass, &Location, &Rotation)`; 0 means allowed.
4. Spawn with spawn-parameter flags `|= 4` through `0x141bc5dc0(World,
   Class, Transform, Params)`. On failure log
   `ServerCreateBuildingActor_Implementation: BuildPreviewMarkerExtraPiece
   didn't build when we tried.`
5. On the new actor: slot `+0x670(bFlag2)`, slot `+0xa90(bMirrored)`, slot
   `+0x880(1, Team)` with the team from `0x1423881c0(PC)`, slot `+0x888(1)`,
   then `0x144211f60(Actor, Transform)` (inferred: finish spawning).

The log string names the function this was inlined from, which confirms the
original `ServerCreateBuildingActor_Implementation` ended in the same spawn
sequence.

**Design for the re-creation** (mine, to be merged into D5's building
section): validate handle, resolve class from
`GameState.AllPlayerBuildableClasses[Handle]`, check and pay the resource
cost from the inventory, run steps 2 to 5 above, and remove overlapping
weaker pieces as the original does (not visible in the client; observe with
the vanilla client). Name the four virtual slots (`+0x670`, `+0xa90`,
`+0x880`, `+0x888`) with `tools/vt.py` on the building actor vtable before
writing the code.

## 3. `ServerAttemptInteract` (read, filtered)

- `UFortControllerComponent_Interaction::ServerAttemptInteract`
  (`0x141d15810`) is a thin wrapper: it takes the controller's pawn (must
  be a Fort pawn), gets a value from `0x141d08ed0(component)` and calls
  `ServerAttemptInteract_Implementation` (`0x141d14c20`, 467 lines).
- The implementation, in order:
  1. pawn null: log `failed for %s - MyFortPawn == nullptr`, stop;
  2. receiving actor null: log `InterfaceActor == nullptr (%d) or
     ReceivingActor == nullptr (%d)`, stop;
  3. get the actor's interact interface (`0x142ec9900` with the interface
     class from `0x1429529d0`); none: same failure;
  4. distance check: the actor's bounds (`0x143f72cc0`) against the pawn.
     Fails when the 2D distance is above `2 * ValidationDistance + bounds
     radius`, or the height difference is above `bounds height + 2 *
     ValidationDistance`. Log `failed for %s - distance checks`;
  5. call the interface's slot `+0x68` with the interaction data; false:
     log `bInteractionSuccessful was false`;
  6. on success: notify a second interface on the actor (class from
     `0x143e98d10`, slot `+0x10`), tell the controller (slot `+0x1bd0`,
     then `0x142442c50`), and for container actors (class from
     `0x1428ea8f0`) call slot `+0xaa8` and, when true, slot `+0xab0(Pawn,
     …)`; then analytics and a per-player record that was not read.

So the RPC itself is complete, as D6 says. What each interactable does when
its interface slot `+0x68` runs (a chest opening and spawning loot, a door
toggling) is in the actor classes and their Blueprints, not here. The loot
spawn for chests is the part to check against D5 (the pickup helpers are
re-created).

## 4. Not re-read in this pass

These are listed as real in D6 section 1 and were not opened again:
`NetMulticast_Athena_BatchedDamageCues` (`0x142334fc0`), `ClientOnPawnDied`
(`0x1423cd300`), `ClientNotifyWon` (`0x1419c6120`) and the other end-of-match
receipts (`0x1419c5ae0` to `0x1419c7db0`). They are client-side receivers;
the server only calls them. Their argument structs are the thing to get
right, and those come from reflection, not from the bodies.

## 5. Open items

| Item | Needed for |
|---|---|
| `OnReadyToStartMatch` past step 7 (team and PlayerState setup) | anything the server does at ready time beyond the bit |
| Names of building actor slots `+0x670`, `+0xa90`, `+0x880`, `+0x888` and controller slot `+0x1a20` | the build re-creation |
| `0x1426b0790`, the only caller of the extra-piece helper | may hold more of the original build flow |
| Re-run the RPC sweep with a rule for "real but only a guard" (a body under about 30 instructions that calls nothing after its checks) | other RPCs may be misclassified the same way as `ServerCreateBuildingActor` |
