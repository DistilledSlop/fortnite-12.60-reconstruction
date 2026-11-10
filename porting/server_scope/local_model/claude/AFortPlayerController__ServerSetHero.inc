// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4d2e0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423ac4d0.
// Single FString parameter.
if (!ServerSetHero_Validate(NewHeroID))
{
	RPC_ValidateFailed(TEXT("ServerSetHero_Validate"));
	return;
}
ServerSetHero_Implementation(NewHeroID);
