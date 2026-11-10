// dep AFortGameStateZone__HandleSurvivorRescued original 0x141ea3fe0
// CONFIDENCE: MEDIUM. Original 0x141ea3fe0. Only the authority counts rescues. The STAT_Inv1PerfHook_GSZ_RNS scope is profiling only and is not ported.
if (Role == ROLE_Authority)
{
	++NumSurvivorsRescued;
	if (PendingRescueCounterIds.Num() > 0)
	{
		PendingRescueCounterIds.Add(NumSurvivorsRescued);
	}
	SendSurvivorRescuedTelemetry(Rescuer, SurvivorName);
}
