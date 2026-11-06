// CONFIDENCE: high
// Native 0x14202c760 returns the first definition description, or empty text.
const UFortItemDefinition* Definition = GetItemDefinitionBP();
const TArray<FText> Descriptions = Definition ? Definition->GetDescription() : TArray<FText>();
return Descriptions.Num() > 0 ? Descriptions[0] : FText::GetEmpty();
