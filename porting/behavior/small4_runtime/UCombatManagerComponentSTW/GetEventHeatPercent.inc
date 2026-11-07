// Original address: 0x1424aa740
// Evidence: CombatEvent=EDX. Value 0x1c -> 0.0f. Else CombatEventHeat[i] (float array at +0x120, stride 4) / Events[i].MaxHeatContribution (FCombatEventData stride 0x38 at +0x1e8, field +4).
// CONFIDENCE: HIGH
const int32 EventIndex = static_cast<int32>(CombatEvent);
if (EventIndex == 0x1c)
{
	return 0.0f;
}
return CombatEventHeat[EventIndex] / Events[EventIndex].MaxHeatContribution;
