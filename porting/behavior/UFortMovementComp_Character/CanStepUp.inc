// Original address: 0x1420d9790
// CONFIDENCE: high
const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (!Player || Player->GetLocalRole() != ROLE_SimulatedProxy)
{
	return Super::CanStepUp(Hit);
}

if ((NativeExtraFlags_1e39 & 2) != 0)
{
	return Super::CanStepUp(Hit);
}
return false;
