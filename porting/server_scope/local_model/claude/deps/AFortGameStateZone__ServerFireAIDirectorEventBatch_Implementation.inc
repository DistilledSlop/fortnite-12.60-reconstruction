// dep AFortGameStateZone__ServerFireAIDirectorEventBatch_Implementation original 0x141eae6f0
// CONFIDENCE: MEDIUM. Original 0x141eae6f0 (vtable +0x950).
if (Role == ROLE_Authority && AIDirector != nullptr && Payload.Num() > 0)
{
	for (const FAIDirectorEvent& Event : Payload)
	{
		AIDirectorDispatchEvent(AIDirector, Event);   // 0x1416570e0
	}
}
