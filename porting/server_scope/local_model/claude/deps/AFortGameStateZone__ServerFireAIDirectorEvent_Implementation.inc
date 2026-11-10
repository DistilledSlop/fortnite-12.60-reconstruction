// dep AFortGameStateZone__ServerFireAIDirectorEvent_Implementation original 0x141eae750
// CONFIDENCE: MEDIUM. Original 0x141eae750 (vtable +0x960).
if (Role == ROLE_Authority && AIDirector != nullptr)
{
	FAIDirectorEvent Event_Data;
	Event_Data.Type = Event;
	Event_Data.Source = Source;
	Event_Data.Target = Target;
	Event_Data.Multiplier = Multiplier;
	AIDirectorDispatchEvent(AIDirector, Event_Data);   // 0x1416570e0
}
