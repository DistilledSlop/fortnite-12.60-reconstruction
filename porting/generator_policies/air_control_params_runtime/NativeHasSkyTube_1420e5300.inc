// CONFIDENCE: HIGH. Original 0x1420e5300.
const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner);
return Pawn && Pawn->CurrentSkyTube != nullptr;
