# Aircraft tick dependency recovery

Source candidate: `OnRep_ReplicatedFlightTimestamp.inc` and `proposed-entries.json`.
The reflected exec `0x1427cfe60` increments the script cursor if non-null and
tail-jumps to `0x1418ce280`. That body performs exactly `FlightElapsedTime -
(ReplicatedFlightTimestamp - FlightStartTime)` and stores at +0x2ec. This is
closed independently of tick; names and offsets agree with reflected aircraft
layout. It is a proposed manifest entry, not promoted or built.

Tick `0x1418d7fb0` remains incomplete. It first checks cast GameState and
native byte +0x2f1. Simulated-proxy role byte 1 performs the cosmetic path using
`0x1419918d0` and `0x1418b99f0`, then tests +0x2f1 again. A negative
FlightStartTime returns; NaN does not. The game-state virtual +0x630 is GetServerWorldTimeSeconds (vtable_slots.json,
slot 198, inherited identity verified) and supplies the timestamp, and role byte 3 assigns FlightElapsedTime = timestamp -
FlightStartTime. It updates ReplicatedFlightTimestamp only when timestamp minus
its previous value is strictly greater than the float at `0x1478c2400`. Other
roles add DeltaSeconds, then unless World.IsPlayingReplay() apply bounded signed
ClientFlightTimerDrift correction controlled by mutable float `0x1478c2360` (captured value 0.33). The timestamp replication
threshold at `0x1478c2400` is 1.0 in this capture; mutability and registrations
still require proof before using constants.
Movement uses FlightInfo.FlightStartRotation.Vector(), FlightSpeed,
FlightElapsedTime and FlightStartLocation, and calls actor SetActorLocation
with sweep=false, hit=null, teleport=false; arithmetic is `(direction * speed)
* elapsed + start`, componentwise. The role-3 interval and end transitions are
documented in aircraft_dropzone_runtime/README.md. Callbacks can mutate +0x2f1,
so the repeated test and broadcast ordering matter.

`0x143f6c200` is AActor::Destroy(bool bNetForce, bool bShouldModifyLevel),
matching UE's implementation; the tick calls Destroy(false, true). The movement
callee `0x143f832a0` is SetActorLocation and `0x142c620f0` matches
FRotator::Vector. `0x144693a30` is World.IsPlayingReplay as previously proved.
Do not promote tick until the cosmetic helper and delegate declaration/binding,
mutable console variables and native +0x2f1 storage
are closed. The cosmetic lookup `0x1419918d0` obtains DefaultBattleBus, may
consult player interface +0x5a8, and falls back to game cosmetics default;
its interface/provider identities remain unresolved. The helper
`0x1418b99f0` is a split-unwind function; its first fragment ends 0x1418b9a25,
so do not truncate there. The preceding client path obtains the first controller,
casts it to Fortnite player controller, calls the lookup and updates/spawns or asynchronously loads the
cosmetic actor when its +0x220 skin differs, then calls actor virtual +0x5f0
with fixed float, true, false if present.

Evidence: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
VA minus 0x140000000; disassembled with `build_vtable_slots.Image.disasm`.
No runtime or build validation is claimed. Tick remains excluded from the manifest.
