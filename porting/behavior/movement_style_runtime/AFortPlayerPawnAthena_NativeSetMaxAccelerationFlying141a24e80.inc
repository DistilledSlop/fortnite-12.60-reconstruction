// CONFIDENCE: HIGH
// Original Athena virtual +0x970.
if (UFortMovementComp_CharacterAthena* Movement = Cast<UFortMovementComp_CharacterAthena>(GetCharacterMovement()))
    Movement->MaxAccelerationFlying = Value;
