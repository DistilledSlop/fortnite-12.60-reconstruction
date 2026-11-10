// CONFIDENCE: MEDIUM
// Original address: 0x142a66660 (exec thunk). Parameter HUDElementTags (FGameplayTagContainer, struct 0x143e9a300) is read
// from the FFrame. The body is FUN_1423e5c70 (0x1423e5c70), ported inline below.
// Resets the listed HUD elements to the user-settings visibility. Nothing happens for an empty tag container.
if (HUDElementTags.Num() == 0)
{
    return;
}
FName UserSettingsKey("UserSettings", 1);   // 0x142cd4ab0
// Find or add the UserSettings entry in the HUD visibility map at controller +0x24e0.
HUDVisibilityMap.FindOrAddEntry(UserSettingsKey);   // hash probe + 0x140b57340 / 0x1420b3720
FHUDVisibilityEntry* Entry = HUDVisibilityMap.GetEntry(UserSettingsKey);
const UObject* HudOwner = GetHUDOwner();   // 0x14237e580
for (const FGameplayTag& Tag : HUDElementTags)
{
    if (HudOwner == nullptr)
    {
        Entry->CopyTagFromSettings(Tag);         // 0x143e96610 (value copy, flag 0)
    }
    else if (HudOwner->IsTagOverridden(Tag))     // 0x141e27160
    {
        Entry->CopyTagFromSettings(Tag);         // 0x143e96610
    }
    else
    {
        Entry->AppendTagFromSettings(Tag);       // 0x143e8b3b0
    }
}
ApplyHUDVisibilityChanges();   // 0x1423da170
