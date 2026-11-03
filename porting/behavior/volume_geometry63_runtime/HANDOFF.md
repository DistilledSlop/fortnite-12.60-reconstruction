# Volume geometry and actor selection

NativeGetBoundsBox at0x141c756c0 is integrated: one live NativeGetLocationBounds
call, origin minus/plus extent, valid FBox. Original complete148-byte body is
pinned. No forced initialization in the bounds-missing path.

ActorGeometry.draft.cpp is UNCOUNTED and uncompiled. It lifts complete geometry
0x141c6a510 and actor selection0x141c7c5c0 using real generated classes, bounds,
virtual GetComponentsBoundingBox, native volume accessor dispatch, and the
selection comparison/call order recovered in actor_volume61_runtime. It
explicitly requires the real weak membership lifecycle; no empty replacement
set is supplied. Recursion-guard storage and all references need final closure.
Draft float predicates and output scratch reuse remain subject to the targeted
assembly ambiguities documented in actor_volume61_runtime, not broad retesting.
The membership-dependent methods must not be promoted merely because accessor
and geometry helpers compile. Raw native C remains in prior evidence folders.
No tests/builds/runtime runs; last tested2040, next3040.
