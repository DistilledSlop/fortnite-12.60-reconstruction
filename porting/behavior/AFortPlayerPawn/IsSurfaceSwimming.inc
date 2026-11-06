// CONFIDENCE: HIGH
// Reflected exec 0x142a7d510; native 0x1423320c0.
return GetCharacterMovement()->MovementMode == MOVE_Custom &&
    GetCharacterMovement()->CustomMovementMode == EFortCustomMovement::SurfaceSwimming;
