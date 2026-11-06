// CONFIDENCE: high
// Native target 0x142033080.
UFortItemDefinition* Definition = GetItemDefinitionBP();
return Definition ? Definition->GetSmallPreviewImage() : TSoftObjectPtr<UTexture2D>();
