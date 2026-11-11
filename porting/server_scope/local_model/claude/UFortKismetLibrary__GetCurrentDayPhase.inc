// CONFIDENCE: MEDIUM. Original exec 0x14296c980; body 0x141ef1ca0 (calls 0x141f611f0, the time-of-day manager lookup; byte field at 0x228 is CurrentDayNightPhase).
// Returns the current day phase of the time-of-day manager resolved from the context, or EFortDayPhase::NumPhases (4) when no manager resolves.
AFortTimeOfDayManager* TimeOfDayManager = AFortTimeOfDayManager::GetTimeOfDayManagerFromContext(WorldContextObject);
if (TimeOfDayManager == nullptr)
{
	return EFortDayPhase::NumPhases;
}

return TimeOfDayManager->CurrentDayNightPhase;
