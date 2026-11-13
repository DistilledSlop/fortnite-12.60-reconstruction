# Aircraft authority Tick subpath and constructor timing

`NativeTickAuthorityFlight.inc` is the complete authority subpath selected from
original `AFortAthenaAircraft::Tick` **0x1418d7fb0..0x1418d838a**. It is a helper,
not a replacement Tick or a recovered complete constructor. The full original
Tick still has an unresolved simulated-proxy cosmetic-loading branch, so this
bundle does not install a Tick override or enable actor ticking. No no-op
cosmetic dependency or client fallback is supplied.

Source evidence: cached `generated/callee_probe/gameplay_rough_pass_1/
0x1418d7fb0.{c,asm,json}`. New disassembly/evidence and an unintegrated complete
Tick call-site draft are under ignored `generated/agents/aircraft-tick-recovery/`.
No Ghidra project was opened. `evidence-contract.json` pins the complete original
Tick and both constructor byte ranges against the late capture image.

## Selected authority contract

The caller supplies the correctly cast Athena GameState and selects authority.
The helper retains the native finished/start guards, queries server world time,
sets elapsed to `Now - FlightStartTime`, and updates the timestamp only when
`Now - ReplicatedFlightTimestamp > interval`. It moves without clamping elapsed
and uses the same multiply order as the original. It then rechecks authority.

Drop-zone membership is recomputed every tick as the strict open interval
`DropStartTime < Now < DropEndTime`. Only an actual membership transition fires
entered/exited. Thus a tick that skips the entire window emits neither event;
it does not synthesize entry and exit. Flight completion uses strict
`Now > FlightEndTime`. It sets finished first, clears/fires any still-active
exit, then fires ended. Callbacks use the actual script multicast delegates
with an aircraft-pointer parameter blob. State is re-read after callbacks.
For out-of-phase aircraft, entry calls existing complete
`AFortGameStateAthena::SetAircraftLocked(false)` (native **0x1419adfe0**), and
completion calls `Destroy(false, true)` after the ended delegate. The original
continues these accesses after callbacks; the driver must avoid fresh access
to a destroyed aircraft after the helper returns.

This differs from the current compatibility driver: clamped motion; a separate
`Run.LastTimestampReplication` clock; `>=` thresholds; sticky entered/exited
flags that manufacture an entry after a missed window; and direct invocation
of the exit compatibility callback. The recovered helper does not take over
that callback's implementation.

## Integrated by the sole Native owner

1. Promote `manifest.proposed.json` through the normal manifest process, marking
   only this authority helper ported. Import `source_policy.py` and call
   `finalize(model, header, cpp)` after the default-subobject policy. The policy
   requires one accepted public helper with the exact source/signature and
   restores mapped constructor defaults inside the existing root-subobject
   constructor. It refuses an existing full Tick until ownership is reconciled.
2. Preserve the existing entered/ended delegate bindings. Bridge
   `NativeAircraftExitedDropZone` to the existing complete MatchFlow exit
   callback through the existing native-lifecycle UFUNCTION hook, with the same
   aircraft parameter. Merely replacing the current direct callback with an
   unbound multicast would silently remove forced-exit/phase behavior.
3. Replace the compatibility driver's per-aircraft motion, timestamp and all
   event/state logic with one helper call. Preserve its weak-key bookkeeping and
   remove completed/invalid entries using validity checks after the call.
   Retire `LastTimestampReplication` and duplicate entered/exited/ended state;
   native aircraft fields become the sole event/timestamp state.
4. Keep the existing single server driver. Do not enable actor ticking from
   `BaseConstructorFlags.inc` until complete original Tick is integrated; an
   inherited AActor Tick would execute Blueprint ReceiveTick, which this
   original native Tick does not call. The flags fragment is evidence only.
5. Use the combined client/server build and affected join/flight checkpoint
   scheduled by the owner; no generation/build/runtime was performed here.

## Constructor and timestamp initialization

Class constructor registration **0x1427cdfa0** supplies thunk **0x1427ce0a0**,
which jumps to the constructor **0x1418b4050**. Its superclass constructor is
**0x141760af0**. The timing/flight constructor statements in
`ConstructorTiming.inc` are mapped using reflected offsets; bool setter
**0x1427cebe0** independently identifies byte **0x2e0** as
`bIsOutOfPhaseAircraft` (decoded bool-bits offset736, mask1, mov8).
The four absolute times and three flight-info duration fields start at -1;
elapsed starts at 0, both event-state bytes at 0. Neither replicated timestamp
nor client drift has an explicit constructor store; UObject zero initialization
supplies their baseline. Do not fabricate server-time initialization there.

Original StartFlightPath **0x1418d5590** writes start and timestamp to its Time
argument, elapsed to zero, and the three absolute end/drop times to Time plus
flight-info duration. It does not reset client drift or call ForceNetUpdate.
Existing MatchFlow retains an additional compatibility ForceNetUpdate call.
The cache's label for this function is incorrect; its body/strings establish
this contract.

The base constructor sets bCanEverTick and bStartWithTickEnabled, prephysics
TickGroup, replication/always-relevant flags, and disables movement replication,
damage and collision. It does not override bAllowTickOnDedicatedServer. Those
facts do not authorize applying tick flags before complete Tick ownership.

## Remaining full Tick closure and current bus issue

Original client Tick increments elapsed by DeltaSeconds and, outside replay,
subtracts `sign(drift) * min(abs(drift), clamp(smoothing,0,1)*DeltaSeconds)`
from both elapsed and drift. Default 0.33 is a bounded correction rate, not a
33-percent lerp. Native replay predicate **0x144693a30** tests the world's demo
net driver using **0x14419d510**; local UE `World->IsPlayingReplay()` is the
corresponding contract. The simulated-proxy branch resolves the local Athena
controller through **0x14468f240**, resolves a bus skin through **0x1419918d0**,
conditionally calls async skin loader **0x1418b99f0**, and calls
`SpawnedCosmeticActor->PrestreamTextures(2.0f, true, 0)` (virtual +0x5f0,
base target **0x143f7f9c0**). The skin resolver's player interface/default-provider
closure and full asynchronous loader ownership still need recovery. Full Tick
is not marked ported.

The original Windows clients already execute that native Tick. Missing generated
client Tick is a reconstruction gap, not proof of the currently observed live
bus offset. No current original-client desynchronization cause or runtime parity
repair is claimed. Image/reflection guards and an in-memory splice check against
the current emitted constructor passed; compilation/runtime remain pending.

The authority helper, constructor policy and exit compatibility bridge are now
integrated into generation. The match-flow timer remains 0.05 seconds, rather
than the original actor Tick cadence. Strict native thresholds are preserved; a
whole drop window skipped between callbacks emits no transition, as in the native
body. This scheduling limitation is not a claim of full Tick parity or a proven
fix for client-to-client bus offsets. Promotion accepted the complete helper
without rejected snippets; the combined build and gameplay pass remain pending.
