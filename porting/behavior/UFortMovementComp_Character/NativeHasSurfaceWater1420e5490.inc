// Original address: 0x1420e5490
// CONFIDENCE: high
const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
return Player && Player->CurrentWaterBody != nullptr &&
       !(MovementMode == MOVE_Custom && (CustomMovementMode == 1 || CustomMovementMode == 2));
