// CONFIDENCE: MEDIUM Tail-call thunk: 0x1415ecbf0 forwards the same arguments to the core, so this variant is the base trajectory plus a hit result output. Profile name is None, so the channel trace is used.
// Original: exec 0x1428f7340 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f7340.c)
CalculateProjectileTrajectoryCore(OutSplinePoints, OutSplineTangents, Instigator, InitialLocation, InitialVelocity, MaxSpeed, Gravity, Friction, Bounciness, TimeStep, TraceExtent, TraceChannel, MaxBounces, MaxSteps, MaxDistanceBetweenSplinePoints, InitialDistance, NAME_None, &OutHitResult);
