# Air-control support

The promoted subset contains the two recovered vortex/SkyTube predicates,
`AFortSkyTube::PostInitializeComponents`, its three non-reflected float caches,
and the original `FAirControlParams` default constructor policy.

SkyTube initialization (`0x1427057c0`, actor vtable slot 144) calls Super, then
evaluates `EntryVelocityAngleRequired`, `SteeringMagnetMaxSteerAngle`, and
`ActiveForwardSteeringAngle` at level zero with null context. It stores their
cosines at original offsets `0x328`, `0x538`, and `0x350`, respectively, using
the exact radians multiplier `0x3c8efa35`. These are cached lifecycle results;
the code preserves authored scalable-float values rather than substituting
constructor tuning defaults. Reconstructed C++ offsets are not an ABI claim.

The struct policy guards the eight original 32-byte scalable-float members,
their order, reflected size, package, and declarations before adding the
constructor recovered at `0x1420d2af0`. Their recovered value is zero with the
scalable-float cache sentinel initialized to minus one by the Engine constructor.

The selector/direction proposals and larger proposed manifest are not promoted.
They still require the original pawn aim-rotation override, vehicle-interface
behavior, and shared movement state. `PhysCustom` is separate incomplete work.
Do not append both proposed manifests or treat these support functions as full
authoritative airborne physics.

Generation accepted 5,693 behavior snippets with zero rejections. Build and
affected gameplay verification follow the next coherent integration batch.
