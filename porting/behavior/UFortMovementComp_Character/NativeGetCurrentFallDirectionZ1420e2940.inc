// Original address: 0x1420e2940
// CONFIDENCE: high
const bool bUseCustomAccel =
    (NativeAirborneFlag != 0) ||
    (MovementMode == MOVE_Custom &&
     (CustomMovementMode == 3 || CustomMovementMode == 4 ||
      CustomMovementMode == 9 || CustomMovementMode == 5 ||
      CustomMovementMode == 12));
const FVector& AccelVec = bUseCustomAccel ? CustomFallingAcceleration : Acceleration;
return FMath::Abs(AccelVec.GetSafeNormal().Z);
