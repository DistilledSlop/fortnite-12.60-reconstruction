// CONFIDENCE: MEDIUM. Returns the ability spec handle that the placement system holds for this deco tool's item-entry GUID; INDEX_NONE when the placement system or the entry is missing.
// Original address: 0x142781450 (exec). Native body: 0x14266f720 (placement system read 0x14269e2e0; entry lookup via placement member slot 0xb0; handle read via entry slot 0x480).
UObject* PlacementSystem = GetPlacementSystem();
if (!PlacementSystem)
{
    return INDEX_NONE;
}

const FGuid Key = ItemEntryGuid;
PlacementEntry* Entry = PlacementSystem->FindPlacementEntryForGuid(Key);
if (!Entry)
{
    return INDEX_NONE;
}
return Entry->GetAbilitySpecHandle();
