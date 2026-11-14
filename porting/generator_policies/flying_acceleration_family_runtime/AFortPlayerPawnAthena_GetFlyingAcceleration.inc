// CONFIDENCE: HIGH
// Original AFortPlayerPawnAthena primary virtual +0x968, 0x141a12f90.
const float PlayerAcceleration = AFortPlayerPawn::NativeGetFlyingAcceleration14231a880();
if (PlayerAcceleration > 0.0f)
    return PlayerAcceleration;
if (AdvancedMovementSet)
    return AdvancedMovementSet->MaxAccelerationFlying.GetCurrentValue();
if (const UFortMovementComp_CharacterAthena* AthenaMovement = Cast<UFortMovementComp_CharacterAthena>(GetCharacterMovement()))
    return AthenaMovement->MaxAccelerationFlying;
return NativeGetPawnStats141a07b70()->MaxAccelerationFlying;
