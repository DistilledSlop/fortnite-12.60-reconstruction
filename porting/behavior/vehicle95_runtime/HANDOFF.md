# Vehicle batch 95 handoff

Three high-confidence candidate bodies in manifest-entries.json, unpromoted and
build pending: HasConstraints (wrapper 0x14283bd70, callee 0x14191ab70),
IsFuelBelowThreshold (0x14283bf40), and IsOutOfFuel (0x14283c0d0). Both fuel
wrappers call 0x141b6a180, a direct load from timer+0xc4, reflected as Charge;
the below-threshold wrapper compares against vehicle+0x6e0 = timer+0xc0,
reflected as ChargeThreshold. ASM COMISS/SETC and COMISS/SETBE report true on
unordered inputs; the negated comparisons preserve that behavior. For
HasConstraints, the diagnostic path in 0x14191ab70 cannot execute: the only
nonzero pointer is selected exactly when JZ skips the call, and XOR AL returns
false. Evidence: porting/generated/native95-broad and native95-impl C and ASM.

Most other Athena exec wrappers are virtual dispatch and require vtable/body
recovery. CurrentGear callee subtracts raw fields +0x578 and +0x7e8; identities
and safe declarations remain unresolved. SpringLengthRatio needs its array
and element bindings; camera rotation and jumping require object/virtual
bindings. CurrentBudget relies on a global and +0x450. SetUnoccupiedAutoDestroy
calls an additional body. These remain uncounted.

The 22 deduplicated direct callees are in /tmp/native95-vehicle-impl.json;
existing porting/generated/native95-impl already covers them. Wheeled movement
is an existing Engine class; direct implementations and matching UE 4.25 code
are in ../UnrealEngine/Engine/Plugins/Runtime/PhysXVehicles. Original wrappers
GetUseAutoGears, SetTargetGear, and SetUseAutoGears select false/empty PhysX
paths; whether the target Engine's build settings select equivalent behavior
requires separate review. Do not generate duplicate game-class methods.

Commands: bounded Python extraction of /tmp/native95-broad.json and wrapper C;
inspection of native95-broad/native95-impl ASM and Generated reflection
headers; generated manifest via Python. No generation, builds or tests; cadence
next approximately 3,040 bodies. Primary agent owns promotion and validation.

## Additional rough lifts

`rough_native_lift.cpp` captures seven newly recovered implementations: the
avoidance enable/registration path, spring ratio array bounds and load, budget
gate, camera rotation byte, jumping virtual/bit, auto-destroy setter/tail-call,
and gear subtraction. These intentionally retain unresolved offset/type/global
and virtual/helper bindings and are not manifest entries. The avoidance order
is taken from ASM (UID clear, cached world load, bit update, optional world
lookup, manager lookup, optional registration); existing UE source confirms the
intended semantic mapping, but exact timing is preserved in the draft. The
raw gear arithmetic uses uint32 to retain machine wrapping.

Other movement setters at 0x1411a5b30–0x1411a5cd0 match existing UE 4.25
implementations for group masks, raw inputs, and input clamping. They belong
to the Engine source rather than generated Fortnite classes; reconciliation of
that existing implementation and build flags remains uncounted. No generator,
test, build, or canonical source edits performed.

Root review confirms unreachable diagnostic branch and exact native epsilon bytes at0x1459c03a0:77cc2b32, matching1.0e-8f. Confidence tags added before promotion.
