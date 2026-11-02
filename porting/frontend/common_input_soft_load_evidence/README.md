# CommonInput synchronous class load and rooting

Bounded original-image source recovery, not a runtime or full settings parity claim.
The sibling `common_input_platform_evidence/README.md` describes the complete
cache-copy and validation sequence. This note closes the load/root effects and
records constructor evidence needed for that sequence.

## Load and diagnostics

`1407d0ea0..1407d105d` accepts an `FSoftObjectPtr` layout and matches the
4.25 `FSoftObjectPtr::LoadSynchronous` in
`UnrealEngine/Engine/Source/Runtime/CoreUObject/Public/UObject/SoftObjectPtr.h:49`:
initial Get, if null and IsPending then `ToSoftObjectPath().TryLoad(nullptr)`,
and final Get. The duplicated Get/resolve instructions implement the inlined
pending check and persistent pointer cache, not additional load requests. At
`1407d0fc1` it calls `142eda260`; that routine's null/subobject branch, recursive
parent load, ordinary `StaticLoadObject` call and redirector traversal match
`FSoftObjectPath::TryLoad` in `Private/UObject/SoftObjectPath.cpp:402`. The
helper ignores TryLoad's direct result and re-resolves the pointer. It contains
no independent warning/log calls. Loading can still emit engine diagnostics:
`TryLoad` uses `StaticLoadObject` with `LOAD_None`, so do not suppress those
engine diagnostics or fabricate a new per-load diagnostic.

Callers validate the loaded object as a UClass and the class as a child of the
relevant class. Use `InputData.LoadSynchronous()` and each copied
`TSoftClassPtr<UCommonInputControllerData>::LoadSynchronous()`; the target typed
wrapper performs precisely those checks (`SoftObjectPtr.h:599`). A failed or
wrong-type result is null. Do not use `.Get()` alone or use TryLoad's return
without the second Get. `14081d830..14081db61` separately logs the literal
`Trying to access unloaded CommmonInputSettings data. This may force a sync load.`
when the final cache-valid byte is false and log verbosity byte is at least 3;
its log call passes verbosity 3 (Warning), then always tail-calls initialization.
No claim is made here about the reconstructed log category identity or all
underlying loader messages.

## Root predicate

At `14081c219..14081c22a`, before loading, the original computes the **signed**
comparison `settings->InternalIndex <= GUObjectArray.ObjLastNonGCIndex` and
retains that bool for both later branches. This is UE's
`GUObjectArray.IsDisregardForGC(settings)` (`UObjectArray.h:824`), not a test
of the loaded class index and not a general root-everything rule. After storing
a successfully typed `InputDataClass`, and after appending each successfully
typed controller class, the true branch locates that class's object-array item
and ORs internal flag `0x40000000` (RootSet); `UObjectBaseUtility.h:189` and
`ObjectMacros.h:542` identify this as `Class->AddToRoot()`. Preserve the captured
predicate and order, including repeated calls for duplicate soft entries. Existing
classes copied from `CommonInputPlatformData[0].ControllerDataClasses` are not
separately rooted by this loop. The array index checks in generated machine code
are object-array access implementation, not an additional success condition.

## Platform data and constructor

Original reflection `struct_blocks.json` block `0x14783d890` and settings block
`0x145a0e320` identify `FCommonInputPlatformData`: size 40, alignment 8; seven
scalar bytes at 0..6, soft-class array at +8 and class array at +24. Settings
contains seven config structs starting +0x50, and transient `CurrentPlatform`
at +0x190. Initializer `14081c1e0` copies specifically the first config element
at +0x50, not a platform-indexed element. Source reflection lists enum bytes at
+1 and +4; the copy uses all seven bytes individually. Existing generated header
lists the scalar declarations in matching order; the actual reflected Bool setter
locations were not independently recovered here.

Original struct-ops factory `14081dc20` installs vtable `145a0f3e0`; constructor
slot points to `14081e5c0`, which writes these bytes (in declaration order):
`00 01 00 01 00 01 00`, and initializes both arrays empty. Thus defaults are
`bSupported=false`, `DefaultInputType=1`, `bSupportsMouseAndKeyboard=false`,
`bSupportsGamepad=true`, `DefaultGamepadType=0`, `bCanChangeGamepadType=true`,
`bSupportsTouch=false`. The struct is non-zero-constructible and has copy and
destructor ops; do not rely on UObject zero initialization for these defaults.
This proves the struct-ops constructor, not the settings object's complete
constructor or every possible later configuration override.

## Reproduction and scope

`windows.json` records SHA-256 for only the listed small instruction/vtable
windows read with `os.pread`; the whole-image digest is a previously established
baseline and was not recalculated. Decode code windows using e.g.:

```
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 1407d0ea0 1407d105d
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 142eda260 142eda386
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 14081c1e0 14081c751
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 14081d830 14081db61
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 14081dc20 14081dc55
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 14081e5c0 14081e5e2
```

`145a0f3e0` is data: its fourth pointer is `14081e5c0`. Reflection extraction
is existing generated evidence; no generated code, native code, UE build or
runtime state changed in this task. Module registration, settings object
constructor beyond this struct and the broader action widget remain outside scope.
