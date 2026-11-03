# Aircraft event and smoothing contracts

These source fragments are candidates for the original owners, not a standalone
implementation or a manifest entry. `AircraftEvents.h.in` uses the engine `FMulticastScriptDelegate` directly,
without a generated delegate type or UHT declaration.
Integrate its members in recovered native layout order. The names are neutral;
the class is not reflected through these members. `Bindings.cpp.in` contains
fragments for the *existing complete owner functions*: do not create new
lifecycle calls or discard other behavior to insert them. Their handlers must
be existing compatible UFUNCTIONs. The mode's aircraft teardown/removal
`0x141952500` removes the three corresponding bindings, so that complete body
must also be preserved. The special-event mutator has its own owner and cannot
be replaced with the game-mode binding.

Original `0x1418d7fb0` passes the address of a one-pointer parameter blob,
containing the aircraft, to `0x1406a9dc0` for each event. The latter copies a
16-byte-per-entry array, resolves each weak object and function name, invokes
UObject's ProcessEvent virtual +0x210 with the blob, and cleans stale entries.
This is the UE script multicast delegate contract, including snapshot dispatch
and callbacks that can change state, not a native C++ multicast delegate.
Original initialization `0x14196b9cf`, `0x14196ba4f`, `0x14196bacf` binds
`AFortGameModeAthena::OnAircraftEnteredDropZone`, `OnAircraftExitedDropZone`,
and `OnAircraftFlightEnded` respectively. The helper `0x1406aa070` appends
(the `AddDynamic` behavior). At `0x141b2865b` onward the special-event mutator
binds its three same-named callbacks; helper `0x1407903b0` checks for an
existing matching object/name before adding (`AddUniqueDynamic`). Binding
fragments alone do not replace either full surrounding function.

`0x1402647a0` registers `Athena.Aircraft.ClientTimeSmoothing` with a pointer
to float `0x1478c2360`, its original help text, and flags 0. The image contains
0.33f there; this is a mutable process-lifetime console variable reference.
`AircraftTimeSmoothing.cpp.in` supplies the source equivalent; the complete
Tick must access that same storage and retain all original side effects.

The timestamp replication threshold at `0x1478c2400` is in the original PE
`.data` section (characteristics `0xc0000040`, readable and writable). The
on-disk bytes at file offset `0x78c1200` are `00 00 80 3f` (1.0f), matching
the late runtime image. A bounded scan of code and static absolute pointers
found only the Tick read and no direct registration or write. Thus a mutable
process-lifetime float initialized to 1.0f is supported as a neutral source
representation, without inventing a console variable. The scan does not prove
there are no indirect writers; integration must preserve mutable storage.

Evidence image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
VA minus `0x140000000`. Addresses above refer to this image. Assembly was read
across split-unwind fragments. This is static reconstruction; no build or
runtime validation is claimed.

## Game-mode callbacks

The reflected registration table maps `OnAircraftExitedDropZone` to exec
`0x1428f04c0`, which calls native `0x141952190`, and
`OnAircraftFlightEnded` to exec `0x1428f0540`, which calls native
`0x141952500`. The flight-ended candidate is complete: it retains the
conditional `AFortGameStateAthena::StaticClass()` call when GameState is
non-null, then if the aircraft argument is non-null removes the first matching
binding for this game mode from each multicast delegate (each remove also
compacts), and calls `Destroy(false, true)`. UE's `Remove(this, FName(...))` implements that single swap-removal and
compaction. The original function names are used directly, so removal does not
require declarations of the other handlers; the candidate does require the
three fields above. Its proposed manifest is not promoted here.

The exited callback is not closed. Existing full decompile is
`porting/generated/callee_probe/blocking/0x141952190.c`; the native body
iterates world player controllers, filters to the original controller class,
calculates the player's aircraft via `0x141990ec0` and `0x141990d90`, checks
the result against the callback aircraft, and checks two bytes of the object
returned by `0x141695900`. Depending on those tests it calls game-mode virtual
+0x778 or logs and may kick via game-session virtual +0x698, with an additional
world/service predicate involving `0x140fc7960`. After iteration it increments
mode +0xd90; if that reaches game-state +0x1ee8, playlist +0x8c0 selects one of
`0x141971aa0(mode, false)` or `0x141971930(mode, false, true)`. A failed
GameState cast is still dereferenced at this stage. The aircraft-index helper
`0x141990ec0`, controller-related `0x141695900`, service predicate, +0x778,
phase helpers and corresponding fields are not source-identified sufficiently
to preserve all effects. No partial body or manifest entry for exit is supplied.
