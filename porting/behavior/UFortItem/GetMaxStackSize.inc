// CONFIDENCE: HIGH
// Recovered item stack/ownership behavior; see HANDOFF.md.
UFortItemDefinition* Definition = GetItemDefinitionBP();
return Definition ? Definition->NativeGetMaxStackSize(AbilityComp) : 1;
