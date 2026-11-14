// Original address: 0x1420e5430
// CONFIDENCE: high
if (NativeHasSurfaceWater1420e5490())
{
	return true;
}
const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
return Player && Player->bIsInWaterVolume;
