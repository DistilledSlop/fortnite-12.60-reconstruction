// CONFIDENCE: high
const UFortItemDefinition* Definition = GetItemDefinitionBP();
return Definition ? Definition->GetDisplayName(bBaseName) : FText::GetEmpty();
