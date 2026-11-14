// CONFIDENCE: HIGH
// Original 0x14237b1d0. Concrete body; shared interface binding is tracked separately.
// Original PC interface +0xa8, 0x14237b1d0; implementation receives controller this.
OutItems.Empty();
if (Cast<UFortWorldItemDefinition>(Definition))
{
    AFortInventory* ItemInventory = WorldInventory;
    if (ItemInventory)
    {
        ItemInventory->NativeFindAllWorldItems(Definition, OutItems, false);
        return OutItems.Num();
    }
    UFortRegisteredPlayerInfo* PlayerInfo = MyPlayerInfo;
    if (PlayerInfo && PlayerInfo->WorldProfile)
    {
        UFortMcpProfileWorld* Profile = PlayerInfo->WorldProfile;
        const FString ProfileKey = Definition->NativeGetProfileKey();
        Profile->NativeFindProfileItems(ProfileKey, OutItems);
    }
}
else if (MainMcpProfile)
{
    // Original dereferences Definition even when it is null on this branch.
    const FString ProfileKey = Definition->NativeGetProfileKey();
    UFortMcpProfileCampaign* Profile = MainMcpProfile;
    Profile->NativeFindProfileItems(ProfileKey, OutItems);
}
return OutItems.Num();
