// CONFIDENCE: high
// Native target 0x14202dd80.
UFortItemDefinition* Definition = GetItemDefinitionBP();
return Definition ? Definition->GetInactivePreviewImage() : TSoftObjectPtr<UTexture2D>();
