// Original address: 0x1419f9ba0
// CONFIDENCE: high
if (Super::ShouldPerformAirControlForPathFollowing())
{
	if (const AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(CharacterOwner))
	{
		if (!AthenaPawn->IsBotControlled())
		{
			return true;
		}
	}
}
return false;
