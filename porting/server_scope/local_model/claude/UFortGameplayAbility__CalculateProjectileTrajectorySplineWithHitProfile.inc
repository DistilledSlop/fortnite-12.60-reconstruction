// CONFIDENCE: MEDIUM Tail-call thunk to the same core as WithHit (0x1415ecd00). The collision profile name replaces the trace channel: the core checks FName_EqualsEName(profile, None) and uses the profile trace when the name is set.
// Original: exec 0x1428f7890 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f7890.c)
// TraceChannel is not a parameter of this variant, so the core receives an unused value in that slot.
CalculateProjectileTrajectoryCore(OutSplinePoints, OutSplineTangents, Instigator, InitialLocation, InitialVelocity, MaxSpeed, Gravity, Friction, Bounciness, TimeStep, TraceExtent, ECC_Visibility, MaxBounces, MaxSteps, MaxDistanceBetweenSplinePoints, InitialDistance, CollisionProfile, &OutHitResult);
