// CONFIDENCE: high; complete captured native branches and typed dependencies.
// Native 0x1420edbc0 uses the updated component's world Z or the largest finite float.
if (UFortMovementComp_Character* FortMovement = Cast<UFortMovementComp_Character>(GetCharacterMovement()))
{
    USceneComponent* Updated = FortMovement->UpdatedComponent;
    FortMovement->FallingStartedZ = Updated ? Updated->GetComponentLocation().Z : MAX_flt;
}
