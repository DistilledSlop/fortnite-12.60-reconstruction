// CONFIDENCE: high
// Native target 0x142033940.
UFortItemDefinition* Definition = GetItemDefinitionBP();
return Definition ? Definition->GetWidePreviewImage() : TSoftObjectPtr<UTexture2D>();
