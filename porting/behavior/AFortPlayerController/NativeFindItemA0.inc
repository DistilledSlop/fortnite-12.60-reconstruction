// CONFIDENCE: HIGH
// Original 0x14237aa80. Concrete body; shared interface binding is tracked separately.
UFortItem* Result = nullptr;
UFortWorldItemDefinition* WorldDefinition = Cast<UFortWorldItemDefinition>(Definition);
if (WorldDefinition)
{
    AFortInventory* SelectedInventory = bInStorageVault ? OutpostInventory : WorldInventory;
    if (SelectedInventory)
    {
        Result = static_cast<UFortWorldItem*>(SelectedInventory->NativeFindWorldItemByDefinition(Definition, false));
    }
    else
    {
        UFortRegisteredPlayerInfo* PlayerInfo = MyPlayerInfo;
        UFortMcpProfileWorld* Profile = PlayerInfo
            ? (bInStorageVault ? PlayerInfo->OutpostProfile : PlayerInfo->WorldProfile)
            : nullptr;
        if (Profile)
        {
            TArray<UFortItem*> Items;
            {
                // The string is destroyed before selecting Items[0], then the
                // complete temporary array is destroyed before final fallback.
                const FString Key = Definition->NativeGetProfileKey();
                const FMcpProfileItemArray* Group = Profile->NativeFindItemsByTemplateKey(Key);
                if (Group)
                {
                    if (Group->Num() > 0)
                    {
                        Items.Reserve(Group->Num());
                    }
                    for (const FMcpProfileItemPtr& Record : *Group)
                    {
                        // Original dereferences the record without a null test.
                        UFortItem* Item = Cast<UFortItem>(Record->ItemObject);
                        if (Item && Item->GetItemDefinitionBP())
                        {
                            Items.Add(Item);
                        }
                    }
                }
            }
            if (Items.Num() > 0)
            {
                Result = Items[0];
            }
        }
    }
}
else if (Cast<UFortAccountItemDefinition>(Definition))
{
    UFortMcpProfile* Profile = nullptr;
    UFortTokenType* Token = Cast<UFortTokenType>(Definition);
    if (!Token)
    {
        Profile = static_cast<UFortMcpProfileCampaign*>(MainMcpProfile);
    }
    else
    {
        switch (Token->ProfileType)
        {
        case EItemProfileType::Common: Profile = static_cast<UFortMcpProfileCommonCore*>(CommonCoreMcpProfile); break;
        case EItemProfileType::Campaign: Profile = static_cast<UFortMcpProfileCampaign*>(MainMcpProfile); break;
        case EItemProfileType::Athena: Profile = static_cast<UFortMcpProfileAthena*>(AthenaProfile); break;
        default: break;
        }
    }
    // Query key even if profile is null; both are owned original temporaries.
    const FString Key = Definition->NativeGetProfileKey();
    {
        TArray<UFortAccountItem*> Items;
        if (Profile)
        {
            Profile->NativeFindAccountItems1421c1800(Key, Items);
            if (Items.Num() > 0)
            {
                Result = Items[0];
            }
        }
    }
}
if (Result)
{
    return Result;
}
return ViewTargetInventory && WorldDefinition
    ? static_cast<UFortWorldItem*>(ViewTargetInventory->NativeFindWorldItemByDefinition(WorldDefinition, false))
    : nullptr;
