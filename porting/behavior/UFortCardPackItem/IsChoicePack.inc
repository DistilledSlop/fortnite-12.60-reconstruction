// CONFIDENCE: high; original native calls the item definition virtual and checks its concrete class.
// Native 0x14254c020 calls virtual GetItemDefinitionBP, then reads byte +0x389 on a matching definition.
const UFortCardPackItemDefinition* Definition = Cast<UFortCardPackItemDefinition>(GetItemDefinitionBP());
return Definition && Definition->bIsChoicePack;
