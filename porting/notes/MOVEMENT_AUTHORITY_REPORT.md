# Movement authority report: slow walking, jump glitches, falling through floors and walls

Date: 2026-11-14. Investigation only; no source, config or running server was changed.

## 1. Short answer

The server is authoritative and it does not simulate the pawn the way the original client does. Whenever the two
simulations disagree, the server overrules the client and the client is snapped to the server's position. There are
three separate reasons the simulations disagree, in order of how much damage they do:

1. **The server has no building collision.** It only collides with the landscape. The client collides with every
   floor, wall and ramp. The server pawn walks through walls and falls through floors, then drags the client with it.
   This is the cause of "glitching through floors and walls", and to a player it looks like gravity pulling them
   through the world.
2. **The server runs stock Unreal movement; the client runs Fortnite's movement.** The original movement component
   replaces about 85 engine movement functions. The server implements 6 of them. Everything airborne (gravity,
   falling test, air control, landing, jump permission) and most ground-velocity logic runs different code on the two
   sides.
3. **The test rig cannot see what you see.** The automated client runs at roughly 5 to 9 frames per second and only
   ever jumped while standing still on open terrain. Your live server session runs with no movement logging at all.
   So nothing recorded so far measures a high-framerate client, a moving jump, or a jump near a building.

Point 3 is why this has been "fixed" several times without being fixed: the acceptance runs passed on a case that
does not resemble real play.

## 2. What was measured

Source: `porting/generated/runs/20261114-movement-acceptance253-1` (server trace `FortMovementPair`, RPC trace
`network-rpc.jsonl`), source commit `b9bcaa45`.

| Fact | Value |
|---|---|
| Client moves received | 2,868 over 442 s |
| Full corrections (`ClientAdjustPosition`) | 99 |
| Short corrections (`ClientVeryShortAdjustPosition`) | 174 |
| When corrections happened | all between 20 s and 75 s, i.e. only while input was being sent |
| Server tick rate | 30 per second |
| Client move interval while moving | 0.13 to 0.22 s (expected 0.05 s) |

### 2.1 The test client is far too slow to be a valid test

The original client is configured to send a move every 0.05 s. In the run it sent one every 0.13 to 0.22 s, which
means its frame time was that long (about 5 to 9 FPS under Wine with software rendering). Unreal caps one move at
0.125 s (`MaxMoveDeltaTime`), on both sides. A 0.2 s frame therefore only moves the pawn for 0.125 s. In the rig,
"550 cm/s" is really about 550 x 0.125 / 0.2 = 344 cm/s of travel. Speed numbers and "all moves acknowledged" results
from this rig do not transfer to your machine.

### 2.2 The server falls through a floor the client is standing on

Run time 22.7 s to 24.6 s, pawn walking in +X at 550 cm/s:

| Server Z | Client mode | Server mode | Z error |
|---|---|---|---|
| -2611.1 | walking | walking | -8.3 |
| -2615.6 | walking | walking | -12.9 |
| -2623.5 | walking | **falling** | -20.7 |
| -2671.6 | walking | falling | -68.9 |
| -2763.5 | walking | falling | -160.7 |
| -2899.1 | falling | falling | (client pulled down) |
| -2994.7 | walking | walking | 0 |

The client was walking on a surface. The server had nothing under it, fell 384 cm to the landscape, and corrected the
client down through the surface it was standing on. Every one of those moves was a full correction.

### 2.3 The server walks through a wall the client is blocked by

Run time 36 s to 40 s, Shift+W. The client stops at X = 182230 (the blocking mesh was later identified by debugger as
`S_Athena_Foundation_Straight66` in level `Apollo_7x12_BusPort_b`). The server keeps walking at 410 cm/s. Result:
24 of 24 moves corrected, each with a 51.25 cm error, which is exactly one server step (410 x 0.125). The client is
teleported forward through the wall one step at a time.

After that, standing still, the client sat 20.95 cm away from the server in Y for 20 seconds. The server sent a short
correction on **every** move (174 in total) and the error never changed. The client is being pushed out of geometry
that does not exist on the server, so the correction can never succeed.

### 2.4 Jumping disagrees even on open terrain

Second jump of the run (stationary, each step 0.125 s, jump speed 630, gravity 2800):

| Step | Client height gained | Server height gained | Server vertical speed after |
|---|---|---|---|
| 1 | +78.7 cm | +56.8 cm | 280 |
| 2 | +56.9 cm | +13.2 cm | -70 |
| 3 | corrected to server | | |

78.7 is exactly 630 x 0.125: the client's first airborne step has no gravity in it. Its second step equals the
server's first. The client is one step "behind" on gravity, so it is 22 cm higher after step 1 and 66 cm higher after
step 2, and both jumps in the run were corrected on every airborne move. The first jump shows the same pattern
(+71.3 = 630 x 0.1133).

I could not identify the exact client code that produces the gravity-free step. The two stock explanations are ruled
out: the client's engine constructor leaves "apply gravity while jumping" on (checked in the disassembly at
`0x144094c88` to `0x14409500b`), and no player pawn Blueprint sets a jump hold time. It therefore comes from Fortnite
code that the server lacks (section 3.2). How large this effect is at high frame rate is unknown; if it is one frame
long it shrinks with frame time, if it is time-based it does not.

### 2.5 Starting to move disagrees

On every start from standstill the first 5 or 6 moves are corrected with a constant 3.84 to 3.89 cm error. The server
splits a 0.125 s move into three sub-steps while accelerating (7.85 cm in the first move); the client covers
11.77 cm, which is one un-split step. Constant-speed walking on flat landscape then matches to under 1 cm. Part of
this is the low frame rate of the rig, but it shows the acceleration path is not the same code.

## 3. Causes

### 3.1 Cause A: no building collision on the server (floors and walls)

Verified from the traces above and from the existing collision probes:

- The server's floor under the pawn was `Landscape_SpawnIsland` in every sample of the run. It never stood on a mesh.
- Buildings live in "foundation" sub-levels. The blocking wall is in `Apollo_7x12_BusPort_b`, which is a child of
  `LF_7x12_Parent2_2`, itself inside `Apollo_POI_LobbyIsland_b`. The server does not load these levels in normal
  operation. An opt-in probe loads only the outer level, not the nested children.
- Your live server (`porting/generated/chrono-server-logs/gameserver.log`) is started without that probe flag, so it
  has landscape collision only.

Consequence: anywhere there is a building, a floor above terrain, a cave piece or a wall, the two sides disagree, and
the server wins.

### 3.2 Cause B: about 79 original movement functions are missing on the server

I compared the original client's three vtables directly in the dumped image:

| Table | Address |
|---|---|
| Engine `UCharacterMovementComponent` | `0x146a30e30` |
| `UFortMovementComp_Character` | `0x1460a98e0` |
| `UFortMovementComp_CharacterAthena` | `0x145e1db40` |

Inside the engine's 353 movement slots, Fortnite replaces **85** (20 of them replaced again by the Athena class).
The server's two movement classes override **6**: `GetMaxSpeed`, `GetMaxAcceleration`, `DoJump`,
`ApplyVelocityBraking`, `UpdateFromCompressedFlags`, `ServerMove_Implementation`. Of 106 distinct original function
addresses in those slots, 97 are not referenced anywhere in the server source.

Four were decompiled and confirmed for this report (captures under `/opt/fortnite/local-model-package/captures`):

| Original | What it does that stock Unreal does not |
|---|---|
| `0x1420e0430` gravity | Stock gravity, multiplied by the pawn's `GravityZScale` attribute and a second player-only attribute, with separate handling for skydive, parachute and other custom modes. The server always uses the plain -2800. |
| `0x1420e5200` is-falling | Also returns true in custom modes 3, 4, 5, 9 and 12 (skydiving, parachute and related). The server says "not falling" in those modes, which changes correction rules and jump gating. |
| `0x1420e5a40` fall velocity | Custom-mode velocity integration with its own terminal speeds; calls stock only for ordinary falling. |
| `0x1420d9260` velocity update | Mode-dependent replacement for the engine's acceleration/friction step. |

Slot names for the rest come from the UE 4.25 header order shifted by three slots. That shift is confirmed at five
known functions (`GetMaxSpeed`, `HandleImpact`, `DoJump`, `GetMaxAcceleration`, `ApplyVelocityBraking`), so names
between slots 125 and 310 are probably right but each must be confirmed by reading the body before porting. The full
list is in the appendix. The groups that matter most:

- **Airborne:** gravity, fall velocity, is-falling, jump permission (Athena), landing, the air-control functions
  (slots 230, 231, 235, 237), walking-off-ledge and catch-air tests.
- **Ground velocity:** velocity update, input acceleration constrain/scale (Athena), braking deceleration, minimum
  analog speed, pre/post movement state updates (Athena).
- **Mode changes:** set movement mode, on-mode-changed (415 decompiled lines), custom physics.
- **Collision response:** impact handling, slide vector, move-component implementation (Athena).
- **Per-frame:** `TickComponent` in both classes.
- **Networking:** client prediction data (Fortnite's own saved-move type), server prediction data, client adjustment
  and smoothing. The client-side ones only matter for the client; the server-side ones decide when corrections fire.

The automated run never exercised most of this: it jumped only while stationary, so air control, landing with
horizontal speed and ledge behaviour have never been compared at all.

### 3.3 Cause C: the acceptance method

- Rig client at 5 to 9 FPS (section 2.1).
- Jumps only tested stationary on open landscape.
- The live server you play on has no `LogNetPlayerMovement`, no pair trace and no RPC trace, so there is no record of
  a single correction from a high-framerate session. Your reports are currently the only data for that case.

### 3.4 Why walking feels slow

The server does reach the right speeds (550 default, 410 with Shift; your live session's pawn tracker shows 410 to
590 on the ground). I found no evidence of a wrong speed cap in the current build. What remains is corrections: each
one rewinds the client and replays its moves, and near any building they arrive on every move (sections 2.2, 2.3).
I cannot prove from existing data that this is the whole explanation on your machine, because no trace of your
session exists. Step 1 below settles it.

Things checked and ruled out: gravity magnitude (-2800 in the staged config, server integrates it exactly), the
`GravityZScale` attribute (1 on the server), the 0.125 s move cap (same on both sides), capsule size, the engine
movement defaults in the client's engine constructor (identical to UE 4.25 for all 75 recovered values), and the
server's movement network settings (they are the original file's values).

## 4. How to fix it

### Step 1. Make the problem measurable (do this first, about an hour)

1. Start the live server (`porting/run_chrono_server.sh`) with the same diagnostics the rig uses:
   `-FortniteProbeMovementDefaults`, `-FortniteNetworkTrace=<file>`, and
   `LogNetPlayerMovement VeryVerbose` in `-LogCmds`. Play one minute: walk on open ground, jump while running, walk
   into a building, walk up a ramp. That gives the first real high-framerate trace.
2. Summarise it with the same three numbers per 5 s bucket used here: moves received, good acks, corrections; plus
   the error vector and both movement modes for every corrected move.
3. Add to the rig's action list: a jump while holding W, a walk off a ledge, and a run up a ramp. Record the rig's
   move interval in the acceptance checkpoint and mark a run invalid if it exceeds 0.05 s while moving.
4. Acceptance target for every later step: on open terrain, zero corrections in a 60 s session of walking, sprinting
   and moving jumps from your client.

### Step 2. Give the server building collision (fixes floors and walls)

1. Load the foundation sub-levels on the server, including nested children, at the authored transforms. The recovered
   foundation selection, level-record and streaming-cache functions (commits `bdf9d2d8`, `b419a651`, `04db8f45`) are
   the right entry point; what is missing is the lifecycle that actually streams the selected levels in and makes
   them visible on a dedicated server.
2. Confirm collision, not just loading: for the known contact at `(182272, -174016, -2944)` the server must return a
   blocking hit on `S_Cave_Inner_Middle` with normal `(-1, 0, 0)`. The earlier probe loaded 679 meshes and still had
   no usable contact, and five collision payloads are rejected as malformed, so check that the cooked collision
   bodies are actually installed for the loaded meshes.
3. Acceptance: repeat sections 2.2 and 2.3. The server floor must be the mesh, the server must stop at the wall, and
   the standing 20.95 cm error must be gone.

### Step 3. Port the original movement functions (fixes jumping and airborne behaviour)

Port from the appendix in this order, building and running the Step 1 test after each group rather than after each
function:

1. Airborne group: `0x1420e0430`, `0x1420e5a40`, `0x1420e5200`, `0x1419c2ad0` (Athena jump permission),
   `0x1420eb9b0` / `0x1419ec080` (landing), `0x1420e00f0`, `0x1420efc20`, `0x1420ee220` (air control),
   `0x1420eee30`, `0x1420e3b30` (ledge / catch air).
2. Ground velocity group: `0x1420d9260`, `0x1420da7a0`, `0x1420ecb40` / `0x1419efe30`, `0x1420e1980` /
   `0x1419d5340`, `0x1420e20f0`, `0x1420f48b0` / `0x1419fff40`, `0x1420f4810` / `0x1419fff00`.
3. Mode group: `0x1420edf10`, `0x1420e6e30`, `0x1420e85a0`.
4. Collision response: `0x1420e2d80`, `0x1420da1d0`, `0x1419dfce0`.
5. `TickComponent`: `0x1420f0970`, `0x1419fdef0`.
6. Server-side networking slots 299 to 310.

While doing group 1, find the source of the gravity-free first jump step (section 2.4): break on the client's
falling-physics function `0x1440bc140` during a jump and read the character's jump-force time (`+0x328`) and the
movement flag byte (`+0x1f0`). If the force time is non-zero, find what writes it.

### Step 4. Optional stopgap so the game is playable meanwhile

Unreal has a supported "trust the client" switch: `ClientAuthorativePosition=true` under
`[/Script/Engine.GameNetworkManager]` in the server's `DefaultGame.ini` (currently `false`), together with setting
`bIgnoreClientMovementErrorChecksAndCorrection` on the player's movement component. With both, the server stops
sending corrections and adopts the client's position each move, so walking, jumping, floors and walls behave as the
client simulates them. It removes cheat protection and hides the real bugs, so it should sit behind a launch flag and
be off in every acceptance run. It is a workaround, not the fix.

## 5. What is and is not established

Established by measurement or by reading the binary: the server falls and walks through geometry the client collides
with; the override count (85 against 6); the four decompiled bodies; the jump and acceleration mismatches in the rig;
the rig's frame rate; the absence of movement logging on the live server.

Not established: the exact client code behind the gravity-free jump step; how large the jump and acceleration
mismatches are at high frame rate; whether corrections fully explain slow walking on your machine. Step 1 answers the
last two directly.

## Appendix: original movement overrides

Slot = index in the original client's vtable. "Name" is the UE 4.25 name after the verified three-slot shift
(the shift is not verified from slot 312 on, so those are left unnamed apart from the one known function). `=` means the Athena class inherits the Fort
function. Server status: only `GetMaxSpeed` (126), `DoJump` (197), `GetMaxAcceleration` (218),
`ApplyVelocityBraking` (274), `UpdateFromCompressedFlags` and `ServerMove_Implementation` (318) exist.

| Slot | Name (4.25 model) | Fort | Athena |
|---|---|---|---|
| 87 | OnRegister | `0x1420e7ad0` | = |
| 103 | TickComponent | `0x1420f0970` | `0x1419fdef0` |
| 125 | GetGravityZ | `0x1420e0430` | = |
| 126 | GetMaxSpeed | `0x1420e1a80` | `0x1419d53d0` |
| 137 | IsInWater | `0x1420e5430` | = |
| 139 | HandleImpact | `0x1420e2d80` | = |
| 140 | UpdateComponentVelocity | `0x1420f4bd0` | = |
| 143 | MoveUpdatedComponentImpl | `0x1440f6a00` | `0x1419dfce0` |
| 146 | ComputeSlideVector | `0x1420da1d0` | = |
| 163 | RequestDirectMove | `0x1420ec5a0` | = |
| 169 | IsFalling | `0x1420e5200` | = |
| 176 | NotifyBumpedPawn | `0x1420e6540` | = |
| 178 | SetMovementMode | `0x1420edf10` | = |
| 179 | OnMovementModeChanged | `0x1420e6e30` | = |
| 186 | ShouldPerformAirControlForPathFollowing | `0x1440ccd40` | `0x1419f9ba0` |
| 188 | ShouldCatchAir | `0x1420eee30` | = |
| 189 | HandleWalkingOffLedge | `0x1420e3b30` | = |
| 192 | UpdateBasedMovement | `0x142113ad0` | = |
| 197 | DoJump | `0x1420dc080` | `0x1419cd830` |
| 198 | CanAttemptJump | `0x1440a1c70` | `0x1419c2ad0` |
| 199 | Launch | `0x1420e5990` | = |
| 200 | HandlePendingLaunch | `0x1420e36c0` | = |
| 202 | JumpOff | `0x1420e5920` | = |
| 203 | GetBestDirectionOffActor | `0x1420ddb90` | = |
| 207 | ComputeOrientToMovementRotation | `0x1420da140` | = |
| 208 | ApplyRequestedMove | `0x1420d7e10` | = |
| 209 | NotifyJumpApex | `0x1420e65a0` | = |
| 210 | NewFallVelocity | `0x1420e5a40` | = |
| 211 | ImmersionDepth | `0x1419dc7a0` | = |
| 212 | CalcVelocity | `0x1420d9260` | = |
| 215 | GetMinAnalogSpeed | `0x1420e20f0` | = |
| 218 | GetMaxAcceleration | `0x1420e1760` | `0x1419d5300` |
| 219 | GetMaxBrakingDeceleration | `0x1420e1980` | `0x1419d5340` |
| 220 | CanStepUp | `0x1420d9790` | = |
| 222 | SetBase | `0x1420eda20` | = |
| 225 | ApplyAccumulatedForces | `0x1420d7880` | = |
| 227 | UpdateCharacterStateBeforeMovement | `0x1420f48b0` | `0x1419fff40` |
| 228 | UpdateCharacterStateAfterMovement | `0x1420f4810` | `0x1419fff00` |
| 230 | GetFallingLateralAcceleration | `0x1420e00f0` | = |
| 231 | ShouldLimitAirControl | `0x1420efc20` | = |
| 235 | ProcessLanded | `0x1420eb9b0` | `0x1419ec080` |
| 237 | SetNavWalkingPhysics | `0x1420ee220` | = |
| 240 | FindBestNavMeshLocation | `0x1420dcbf0` | = |
| 241 | ShouldComputeAccelerationToReachRequestedVelocity | `0x1420ef210` | = |
| 243 | UnCrouch | `0x1420f3f30` | = |
| 244 | CanCrouchInCurrentState | `0x1420d9600` | = |
| 247 | CheckFall | `0x1440a2970` | `0x1419c40d0` |
| 248 | PhysicsRotation | `0x1420eada0` | = |
| 252 | UpdateProxyAcceleration | `0x1420f5bb0` | = |
| 257 | VisualizeMovement | `0x1420f7a30` | = |
| 263 | PhysNavWalking | `0x1420e8eb0` | = |
| 266 | PhysCustom | `0x1420e85a0` | = |
| 273 | HandleSlopeBoosting | `0x1420e38d0` | = |
| 274 | ApplyVelocityBraking | `0x1420d8430` | = |
| 278 | ComputeFloorDist | `0x1420d9c00` | = |
| 281 | IsValidLandingSpot | `0x1420e58c0` | = |
| 287 | ConstrainInputAcceleration | `0x1420da7a0` | = |
| 288 | ScaleInputAcceleration | `0x1420ecb40` | `0x1419efe30` |
| 292 | FindNavFloor | `0x1420dd270` | = |
| 294 | SimulatedTick | `0x1440cdcc0` | `0x1419fa8e0` |
| 295 | SimulateMovement | `0x1420efd80` | = |
| 299 | SendClientAdjustment | `0x1420f00b0` | = |
| 301 | SmoothCorrection | `0x1420d9a80` | = |
| 302 | GetPredictionData_Client | `0x1420d9420` | = |
| 304 | HasPredictionData_Client | `0x1420ed260` | = |
| 305 | HasPredictionData_Server | `0x1420ed2b0` | = |
| 306 | ResetPredictionData_Client | `0x1420ed980` | = |
| 308 | SmoothClientPosition | `0x1420f4c90` | `0x141a02b20` |
| 310 | ClientUpdatePositionAfterServerUpdate | `0x1420defb0` | `0x1419d3120` |
| 314 | unnamed (network / RPC region) | `0x14097d780` | = |
| 316 | unnamed (network / RPC region) | `0x1406801c0` | = |
| 317 | unnamed (network / RPC region) | `0x1420ed300` | = |
| 318 | ServerMove_Implementation | `0x1420ed7e0` | = |
| 320 | unnamed (network / RPC region) | `0x1420ed440` | = |
| 326 | unnamed (network / RPC region) | `0x1420ed620` | = |
| 327 | unnamed (network / RPC region) | `0x1420ed6c0` | = |
| 339 | unnamed (network / RPC region) | `0x1420e6b70` | = |
| 344 | unnamed (network / RPC region) | `0x1420d91f0` | = |
| 345 | unnamed (network / RPC region) | `0x1420eb0a0` | = |
| 347 | unnamed (network / RPC region) | `0x1420f6bb0` | = |
| 348 | unnamed (network / RPC region) | `0x1420efd20` | = |
| 349 | unnamed (network / RPC region) | `0x1420f4300` | `0x1419ffe60` |
| 350 | unnamed (network / RPC region) | `0x1420ef960` | `0x1419f98d0` |
| 351 | unnamed (network / RPC region) | `0x1420df170` | `0x1419d3360` |
| 352 | unnamed (network / RPC region) | `0x1420ec7e0` | `0x1419ee300` |
