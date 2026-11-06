// WorldItem virtual override 0x14205ca30.
// CONFIDENCE: HIGH
const UFortWorldItemDefinition* Definition = Cast<UFortWorldItemDefinition>(GetItemDefinitionBP());
return Definition && Definition->bItemHasDurability;
