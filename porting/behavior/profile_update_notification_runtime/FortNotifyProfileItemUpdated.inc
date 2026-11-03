// CONFIDENCE: HIGH
// 0x1421e3570, UFortMcpProfile's +0x2a8 override.
{
    Super::NotifyProfileItemUpdated(Item);

    FString Type;
    FString Name;
    FPrimaryAssetId OriginalId;
    if (Item.TemplateId.Split(TEXT(":"), &Type, &Name))
    {
        OriginalId = FPrimaryAssetId(FPrimaryAssetType(FName(*Type)), FName(*Name));
    }

    const FPrimaryAssetId RedirectedId = UFortAssetManager::NativeGetFortAssetManager().GetRedirectedPrimaryAssetId(OriginalId);
    if (!RedirectedId.IsValid() || RedirectedId == OriginalId)
    {
        return;
    }

    Item.TemplateId = RedirectedId.ToString();
    int32 Separator = INDEX_NONE;
    Item.AlternateDefinitionKey = Item.TemplateId.FindChar(TEXT(':'), Separator)
        ? Item.TemplateId.Left(Separator) : Item.TemplateId;
}
