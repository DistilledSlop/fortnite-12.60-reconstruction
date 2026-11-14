# Athena movement constructor: reflected-field projection

Original class constructor is `0x1419b9480` (GPSC -> InternalConstructor -> constructor; see `porting/tools/ctor_defaults.py`). It calls `UFortMovementComp_Character` constructor at `0x1419b9497`, installs vtables, then initializes the native `JumpPenalties` array at `this+0x1e40`. Stores at `0x1419b94ee..0x1419b95a4` write five consecutive 8-byte elements: `(1,1), (1,1), (.75,.75), (.5,.5), (.25,.25)`. The reflection struct declares float `JumpScalar` at +0 and float `MovementScalar` at +4. The array uses normal TArray growth calls; capacity is not observable default data.

The following 4-byte stores are typed using reflection, class inheritance, and the generated parent and UE 4.25 CharacterMovementComponent declarations (all inherited fields below are public):

| Address | Offset | Field | Value |
| --- | --- | --- | --- |
| 0x1419b95bd | 0x1e50 | JumpPenaltyResetTime | float 1 (8-byte store also zeroes +0x1e54) |
| 0x1419b95c8 | 0x2d0 | NetworkMaxSmoothUpdateDistance | float 768 |
| 0x1419b95d2 | 0x2d4 | NetworkNoSmoothUpdateDistance | float 1024 |
| 0x1419b95dc | 0x2b8 | NetworkSimulatedSmoothLocationTime | float bits 0x3e19999a (0.15f) |
| 0x1419b95e6 | 0x650 | NetworkSmoothingVisibilityThreshold | float 1 |
| 0x1419b95f0 | 0x1e60 | NetworkSkipSkyDivingCollisionForPawnLOD | int32 2 |
| 0x1419b95fa | 0x4d8 | MinTimeBetweenTimeStampResets | float 1200 |
| 0x1419b9604 | 0x1b0 | BrakingSubStepTime | float bits 0x3c888889 (1.0f / 60.0f) |
| 0x1419b961d | 0x1e64 | MaxAccelerationFlying | float 1000 |

This policy projects the array, these nine reflected scalars and the inherited server dual-move flag. `0x1419b960e..0x1419b9627` copies bit 0 from registered int32 `Athena.ServerEnableDualMoveScopedMovementUpdates` at `0x1478c2860` into inherited `bEnableServerDualMoveScopedMovementUpdates` at offset 0x1f1 using RMW. Registrar `0x140274900` binds the storage with flags 0; its exact help includes a trailing newline. The on-disk PE initializes the int32 to 1. Generated support registers the same setting and the constructor reads its low bit; console changes affect later constructions. `native-evidence.json` pins the registrar, constructor and PE initializer. This does **not** represent all constructor behavior. Stores at `0x1419b95b1` (zero qword +0x1e58), `0x1419b962d` (float bits 0x3f800000 at +0x1e74), `0x1419b963f` and `0x1419b964d` (12 bytes from global data at +0x1e68), and `0x1419b9656` (byte 1 at +0x1cd8) do not map to reflected fields in the available class chain. The +0x1e54 zero is likewise unmapped. Their identities and semantics remain unresolved; do not infer values for fields or claim full class-local constructor equivalence. Native array allocation and base constructor side effects follow the corresponding C++ operations.
