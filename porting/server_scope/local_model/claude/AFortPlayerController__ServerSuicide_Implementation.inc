// CONFIDENCE: MEDIUM
// Original: 0x1423acfe0 (virtual Implementation of ServerSuicide). Callees not named here are described in the evidence file.
// Suicide, unless the pawn was just spawned and the grace period applies.
if (APawn* Pawn = GetPawn())
{
	if (GetWorld()->TimeSeconds - Pawn->NativeRecoveredFloat_0x9c <= 1.0f && NativeIsSuicideGracePeriodActive())
	{
		return;
	}
	if (AFortPlayerPawn* FortPawn = Cast<AFortPlayerPawn>(Pawn))
	{
		FortPawn->NativeSuicide();
	}
}
