// Native 0x142385770. CONFIDENCE: high
AFortPlayerPawn* Pawn = MyFortPawn;
if (!Pawn || !Pawn->GetCharacterMovement())
{
    return false;
}
return Pawn->GetCharacterMovement()->bUseControllerDesiredRotation;
