// Original address: 0x1420e5200.
// CONFIDENCE: HIGH
// The engine predicate remains authoritative for ordinary falling.
return Super::IsFalling() ||
    (MovementMode == MOVE_Custom &&
     (CustomMovementMode == 3 || CustomMovementMode == 4 ||
      CustomMovementMode == 5 || CustomMovementMode == 9 ||
      CustomMovementMode == 12));
