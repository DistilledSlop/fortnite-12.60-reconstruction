// CONFIDENCE: HIGH
UFortItemDefinition* Definition = GetItemDefinitionBP();
return Definition ? Definition->NativeGetItemType() : EFortItemType::Max_None;
