# Aircraft drop-zone callback and producer investigation

This is static reconstruction, not a runtime or build validation. The callback
fragment is complete against the previously recovered GetAircraft and
SetAircraftLocked bodies. The proposed manifest at
`/tmp/gameplay-aircraft-dropzone-writer/proposed-manifest.json` is deliberately
not promoted: the aircraft tick producer and its callback wiring remain open.
The field spelling is reconstructed.

The callback casts GameState and intentionally dereferences a failed cast; it
checks every index, rereading the array count, and calls GetAircraft even after
one aircraft fails the test. Null elements do not fail it, so empty and all-null
arrays unlock. GetAircraft's phase-dependent logging, replay and world calls
must remain intact. The reflected exec `0x1428f0440` consumes one
AFortAthenaAircraft* and passes it to `0x1419520f0`, whose body ignores it.

## Native byte producer

The AFortAthenaAircraft constructor `0x1418b4050` zeros EDI at `0x1418b4062`
and writes DI to +0x2f0 at `0x1418b428f`, initializing both +0x2f0 and +0x2f1
to zero. It also initializes three 16-byte containers at +0x260, +0x270, and
+0x280 to zero. The tick routine starts at `0x1418d7fb0`: it obtains World,
casts World+0x130 to AFortGameStateAthena and returns on failed cast or when
+0x2f1 is nonzero. It contains additional client cosmetic, time drift, movement,
and completion effects; they cannot be removed to produce a standalone body.

At `0x1418d80f3` it calls game-state virtual slot +0x630 and preserves the
float result. When actor role byte +0xf0 is 3, at `0x1418d8272` it sets byte
+0x2f0 to 1 exactly when that float is strictly greater than DropStartTime
(+0x2d8) and strictly less than DropEndTime (+0x2dc), otherwise 0. The
comparisons also yield false for unordered floats. It saves the previous whole
byte before the assignment. On zero-to-nonzero it broadcasts the +0x260
container via `0x1406a9dc0`, passing this aircraft, then if reflected
bIsOutOfPhaseAircraft (+0x2e0) is nonzero calls SetAircraftLocked(false). On
nonzero-to-zero it broadcasts the +0x270 container. In either case it next
checks +0x2f1; if zero and the float is strictly greater than FlightEndTime
(+0x2d4), it writes +0x2f1 = 1, and if +0x2f0 is nonzero clears it and
broadcasts +0x270, then broadcasts +0x280. If bIsOutOfPhaseAircraft is set it
calls `0x143f6c200(this, false, true)` after the final broadcast. The float
comparison at the end similarly rejects unordered values.

The +0x260/+0x270/+0x280 broadcasts are consistent with the entered, exited,
and ended events respectively; establishing their precise native declaration,
binding site, and the +0x630 virtual's source contract requires further work.
The smallest remaining meaningful implementation boundary is the complete tick
`0x1418d7fb0`, including its client cosmetic helper `0x1418b99f0`, time/position
updates, broadcast mechanism, and destruction callee `0x143f6c200`; producing
only the interval assignment would silently omit observable effects. The
+0x2f1 byte is the end-of-flight guard, separately initialized in the same
word store. Do not add an invented callback invocation or tick override to the
manifest until those contracts and the registration are closed.

Evidence image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`
uses file offset VA minus `0x140000000`. Window hashes: constructor
`0x1418b4050`, length `0x282`, SHA-256
`0cf9f3dee753c73d169095c2d48684a94f2db2f53505731549f80065ce2b1fa6`;
tick `0x1418d7fb0`, length `0x3d7`, SHA-256
`ad43caad0cd2dd996e57d6822beb6844a65f54b1f1ae9e7b31d038b0cd5c2f51`;
exec `0x1428f0440`, length `0x80`, SHA-256
`823994232442f651fb7709be0cafe397473bb04f5b98ce0b238f48cb6ba9328a`.
Disassembly used `build_vtable_slots.Image.disasm` and the image's unwind
function extents; tick has split unwind fragments and was read as the full
contiguous range rather than truncating at the first fragment. See also
`aircraft_dropzone_evidence.md` and `aircraft_lookup_evidence.md`.
