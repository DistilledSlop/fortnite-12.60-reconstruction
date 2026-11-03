// Native 0x1423b0960. CONFIDENCE: high
AFortPlayerPawn* Pawn = MyFortPawn;
if (Pawn && Pawn->GetCharacterMovement())
{
    Pawn->GetCharacterMovement()->bUseControllerDesiredRotation = bNewRotatePawnToCamera;
}
