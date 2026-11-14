// CONFIDENCE: HIGH
// Original UFortMovementComp_CharacterAthena::GetMaxAcceleration, 0x1419d5300.
if (MovementMode == MOVE_Flying)
    return static_cast<const AFortPawn*>(CharacterOwner)->NativeGetFlyingAcceleration14231a880() * MaxAccelerationFlyingMultiplier;
return UFortMovementComp_Character::GetMaxAcceleration();
