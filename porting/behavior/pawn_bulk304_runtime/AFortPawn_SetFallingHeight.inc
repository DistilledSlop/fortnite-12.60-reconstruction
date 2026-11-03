// CONFIDENCE: high; complete captured native branches and typed dependencies.
// Native 0x14234cbd0 updates the reflected FallingStartedZ only for Fort character movement.
if (UFortMovementComp_Character* FortMovement = Cast<UFortMovementComp_Character>(GetCharacterMovement()))
{
    FortMovement->FallingStartedZ = FromHeight;
}
