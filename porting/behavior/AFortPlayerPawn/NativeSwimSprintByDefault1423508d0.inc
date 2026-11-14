// CONFIDENCE: HIGH
// Original 0x1423508d0: CharacterMovement is deliberately unchecked.
return NativeSwimSprintByDefaultCVar148157d00() != 0 &&
    GetCharacterMovement()->MovementMode == MOVE_Custom &&
    GetCharacterMovement()->CustomMovementMode == 10;
