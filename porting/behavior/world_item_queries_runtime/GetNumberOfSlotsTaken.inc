// WorldItem virtual override 0x14205af00.
// CONFIDENCE: HIGH
const UFortWorldItemDefinition* Definition = Cast<UFortWorldItemDefinition>(GetItemDefinitionBP());
return Definition ? Definition->NumberOfSlotsToTake : 1;
