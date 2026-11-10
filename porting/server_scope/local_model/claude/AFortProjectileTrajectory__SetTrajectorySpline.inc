// CONFIDENCE: MEDIUM. Exec thunk: reads two TArray<FVector>-typed params (SplinePoints first, then SplineTangents, the reverse of the inventory order) and forwards them to the native implementation via vslot 0x628 (-> 0x1426e0280). Both arrays are released afterwards in the thunk.
// Original address: 0x142aac870 (exec). Implementation: 0x1426e0280 (deps AFortProjectileTrajectory__SetTrajectorySpline__Implementation.json).
TArray<FVector> SplinePoints;
TArray<FVector> SplineTangents;
ReadParamFromFrame(SplinePoints);
ReadParamFromFrame(SplineTangents);
SetTrajectorySplineImpl(SplinePoints, SplineTangents);
