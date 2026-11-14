// Original address: 0x1420f4810
// CONFIDENCE: high
Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (Player && Player->GetLocalRole() != ROLE_SimulatedProxy && Player->bIsSlopeSliding)
{
	if (UWorld* World = GetWorld())
	{
		if (World->GetTimeSeconds() - NativeSlideStartTime_1e1c > SlideTimeUntilReset)
		{
			Player->NativeSetIsSlopeSliding14234dae0(false);
		}
	}
}
