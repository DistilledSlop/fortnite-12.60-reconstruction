# FFortAssets owner source materialization

`FortAssets.h` and `FortAssets.cpp` define one function-static owner and compose
`FFortAssetLoadDiagnosticState`, the same substate passed to
`UFortAssetManager::NativeHandleLoadDiagnostic`. Copy both support files into the
same source module as that callback and state. Integrating the files does not
itself instantiate the singleton; recovered callers must use `Get` and the
owner methods. No second cache or registration is introduced.

Evidence: original image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base `0x140000000`, SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`.

* Singleton `141df61e0`, constructor `141de4b00`, destructor `141de77f0`: function-static owner, raw PreLoadMap and synchronous-load bindings with true payload; remove all four delegates in native order. The callback delegates to the recovered body `141e09a30` with the owner's state.
* `141de9e00` track, `141de9e80` GC, `141de9f00` named reference, `141e0de80` unlocked post-startup removal, `141dedeb0` startup flag setter, `141dfc1e0` preload clear. Track admits null; named-reference helper rejects it. GC visits the named-reference keys after unlocking; insertion and individual removal likewise intentionally lack a lock. Local engine pointer hash (`Core/Public/Templates/TypeHash.h`) shifts by four and mixes; existing native evidence identifies the same policy for both pointer sets and the named-reference map. Local `FReferenceCollector` map overload visits only pointer keys when value is `FName`.
* Changed-submode release remains an unported caller block within141deca00; no independently callable release trigger is introduced. Existing private clear is used only by the original PreLoadMap callback. Startup state-machine callers remain open.

The +10/+60 collections are read-identified as `FSoftObjectPath` / `int32`, and their empty initialization and destruction are evidenced. They are declared as empty maps solely to model their owned lifetime; no code invokes their hash policy or mutates them. Their native insertion, equality, and hashing remain unproven: do not integrate map writers or claim read/mutation parity until recovered. Default `TMap` key policy is not evidence of the native policy. See `../asset_diagnostic_maps_runtime/HANDOFF.md`. Native offsets identify evidence only: this source owner and the shared diagnostic struct do not claim native binary layout. Root identified both remaining original vtable slots and integrated the independently recovered referencer-name/property-name virtuals from asset_gc_referencer_virtuals_runtime.

No canonical edits, build, tests, or runtime execution. Verify inclusion of the support files in the module, callback accessibility, and callsite wiring during integration. Full owner callers and release triggers are not yet ported; merely compiling this owner does not complete retention behavior.

Root review: diagnostic maps now use the SAME shared FFortAssetDiagnosticMap alias as read helpers. Two recovered GC diagnostic virtuals preserve name text, lock scope, set precedence, named-map snapshot and unchanged output on failure.
