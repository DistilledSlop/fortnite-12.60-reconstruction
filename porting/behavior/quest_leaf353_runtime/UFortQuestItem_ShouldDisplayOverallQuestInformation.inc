// CONFIDENCE: HIGH; native 0x142451290 reads the definition bit when present.
const UFortQuestItemDefinition* Definition = static_cast<const UFortQuestItemDefinition*>(ItemDefinition);
return Definition && Definition->bShouldDisplayOverallQuestInformation;
