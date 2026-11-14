// CONFIDENCE: high
// Inlined selection shared by original 0x1420e4740/0x1420e4ab0/0x1420df7e0.
const bool bCustomAir = MovementMode == MOVE_Custom &&
    (CustomMovementMode == 3 || CustomMovementMode == 4 || CustomMovementMode == 5 ||
     CustomMovementMode == 9 || CustomMovementMode == 12);
return (NativeAirborneFlag || bCustomAir) ? CustomFallingAcceleration : Acceleration;
