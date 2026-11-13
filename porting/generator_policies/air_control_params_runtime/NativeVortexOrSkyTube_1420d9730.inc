// CONFIDENCE: HIGH. Original 0x1420d9730.
const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner);
return Pawn && (Pawn->bIsInVortex || Pawn->CurrentSkyTube != nullptr);
