// CONFIDENCE: high
// Native 0x142032820: resolve the definition on every call.
const UFortItemDefinition* Definition = GetItemDefinitionBP();
return Definition ? Definition->NativeGetRarity() : EFortRarity::Uncommon;
