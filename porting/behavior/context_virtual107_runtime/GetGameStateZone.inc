// CONFIDENCE: HIGH
// 0x144ccc7a0 dereferences the virtual GetWorld result before the cast.
return Cast<AFortGameStateZone>(GetWorld()->GetGameState());
