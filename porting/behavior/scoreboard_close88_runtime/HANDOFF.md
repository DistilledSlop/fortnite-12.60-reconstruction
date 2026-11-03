# Scoreboard dependency closure 88

`weak_flush.h` is an uncompiled, uncounted typed implementation of the callback
0x141e51ca0, conditional on binding Writer::Flush to the actual writer operation.
It is not a manifest entry or whole AddScoreboardInfoEntry implementation. The
callback captures a two-word weak pointer, conditionally acquires a strong
reference, calls flush only when the captured object pointer is nonnull, then
releases the strong reference. Controller +8 is shared count and +12 weak
count; ordinary increment/decrement, virtual DestroyObject at +0 and deleting
controller at +8 match `FReferenceControllerOps<ESPMode::NotThreadSafe>` in
UE 4.25 `Templates/SharedPointerInternals.h`. `TWeakPtr::Pin()` expresses that
ownership and its expiration behavior without reproducing raw controller ABI.
The original does not perform atomic shared/weak reference operations; do not
silently change the mode to ThreadSafe.

0x141df4ff0 reads writer's first pointer, and if nonnull calls virtual +0x1b0
on its +8 subobject. This is the operation required as Writer::Flush above;
the allocation/subobject type is not yet established, so no concrete class or
field declaration is proposed. 0x141e100c0 labels the object AsyncFileWriter:
it lazily opens the file, creates a 0x108 wrapper, initializes a BOM, serializes
and optionally flushes. It also has a guarded recursive path. Opening,
serialization, allocator and tracing remain unresolved; implementing an empty
or direct archive write would omit behavior. The callback itself is therefore
source-recovered but cannot yet be safely promoted or called in this tree.

UE `FGraphEvent::IsComplete` checks its closable subsequent list and
`FGraphEventRef` uses atomic AddRef/Release with recycle, matching the event at
self+0x3f8 and native +8 bit 26 / +0x48 counter respectively. UE
`FFunctionGraphTask` stores a `TUniqueFunction<void()>`, a desired thread and
returns its completion event through `CreateAndDispatchWhenReady`; these match
the task shape. The value 2 for desired thread must be interpreted against the
actual build's enum before binding; task construction, stat id, exact capture
copy/destruction ordering and receiver class are not yet proven. The lambda
stores a weak writer pointer, not a raw owner or strong writer. Calls in the
main body increment weak count twice, then release one local weak reference
after dispatch. Do not assume asynchronous use makes the original weak mode
thread-safe. The copied trailing eight bytes in the 32-byte small function storage are
uninitialized in the caller and appear to be unused storage; this still needs
closure layout confirmation before promotion.

Caller 0x1418e4610 obtains an object from 0x143f76130, checks its class
against 0x1428ea7f0, and passes that same object as receiver. The corrected
class-getter mapping in `porting/behavior/building_vitals_runtime/README.md`
identifies 0x1428ea7f0 as UFortGameInstance: the receiver is UFortGameInstance
or a subclass. This does not establish field declaration ownership or
construction/destruction, so no generated fields are proposed. This folder intentionally adds no rough main
body, adapters, manifest, tests, canonical code or generation changes.

Source inputs (SHA-256): callback C f1d4fcfb9a5174c5c554c7afa7c501ab24eb64570cd491f235fef9ddb619b4dd, asm fe66bae3f26a6b810e806bfec8259d459b5c39f7f31a8cc2ff5f6ee32aaac2f5; serialize C 5a460380658c42fef6ef83e0326c549f559754414184f16c8dcbe3d43fce9edd, asm ce0fe3205721149d97f77b25bd93bbffa7229eeecb5438312519f2ffaf4d9e28; flush C 45ff2346abadb22a58e8782b0a7562684af0c6614880aedaecacc04c28cfa295, asm 7936f00c634db00d6d961428fb5dfbdac85f55c00336a2c4d7ee0cd90555b575. Inputs at `porting/generated/native88-dependencies/`; main-body pins in `scoreboard_native87_runtime/HANDOFF.md`. Read decompilation, callback/flush assembly and matching UE task/shared-pointer headers. No build or tests, per checkpoint cadence; 2,147 bodies and 221 fields remain accepted, last tested 2,040, next about 3,040.
