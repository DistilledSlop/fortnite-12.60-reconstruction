# Aircraft progress batch: selection audit

No new body is proposed. The aircraft direct candidates in the existing probes
are already in the canonical manifest, or have unclosed dependencies. This audit
uses the static late-capture evidence; it does not claim runtime validation.

* AFortAthenaAircraft's reflected getters `0x1427cfc50`, `0x1427cfc80`, and
  OnRep `0x1427cfe60` are already in `porting/behavior/MANIFEST.json`.
* The aircraft event consumers OnAircraftEnteredDropZone `0x1428f0440` and
  OnAircraftFlightEnded `0x1428f0540`, and the lock helper `0x1419adfe0`, are
  also canonical. Their native body contracts and original image window pins
  are in `aircraft_dropzone_runtime/README.md`,
  `aircraft_events_runtime/README.md`, and `aircraft_unlock_runtime/README.md`.
* The remaining Tick `0x1418d7fb0` is not independent: its simulated-proxy
  path calls `0x1419918d0` and the full cosmetic loader `0x1418b99f0`. The
  player-interface/default-provider bindings, asynchronous load ownership and
  stack argument remain open. The candidate and complete blockers are in
  `aircraft_rough_runtime/README.md`; the original tick 0x3d7-byte window is
  pinned in `aircraft_dropzone_runtime/README.md`.
* The adjacent WorldSettings helper `0x141fd72d0` calls the substantial spawn
  and selection function `0x141fbd1f0` on both paths. It cannot be source
  closed without that helper. In particular its null-manager path discards
  the result; substituting a setter would change observable behavior. See
  `aircraft_larger_recovery_runtime/README.md` and original decompiles in
  `porting/generated/callee_probe/aircraft_larger_recovery{,2}/`.

No build, test, Ghidra launch, or runtime execution was performed. No partial
body or guessed dependency is represented as a manifest entry.
