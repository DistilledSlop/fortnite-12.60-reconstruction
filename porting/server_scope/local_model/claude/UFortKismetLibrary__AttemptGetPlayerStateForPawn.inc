// CONFIDENCE: MEDIUM. Returns the player state that owns FortPawn, or nullptr. Delegates to the shared GetFortPlayerStateForPawn lookup: the pawn's own AFortPlayerState when set, otherwise resolved through the pawn's controlled remote pawn or controller chain.
// Original address: 0x142966e10 (exec). Native helper: 0x141ee5f70 (GetFortPlayerStateForPawn; existing dep AFortPlayerPawn__GetCharacterBodyType__FUN_141ee5f70).
return GetFortPlayerStateForPawn(FortPawn);
