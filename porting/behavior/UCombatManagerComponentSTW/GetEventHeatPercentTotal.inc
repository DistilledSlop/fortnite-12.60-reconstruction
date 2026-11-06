// CONFIDENCE: high. Native target 0x1424aa7d0.
return CombatEvent == EFortCombatEvents::Max_None ? 0.0f : CombatEventHeat[CombatEvent] / CurrentTotalHeat;
