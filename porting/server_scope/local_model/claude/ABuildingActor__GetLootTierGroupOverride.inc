// CONFIDENCE: LOW. Decompile-derived. Scans the world loot-tier override table for an entry whose building tag matches one of this building's gameplay tags and returns that entry's loot tier group. Returns NAME_None when nothing matches.
// Original address: 0x1427370d0 (exec). Native body: 0x141ba7040. Table layout inferred from 16-byte entries (tag FName, group FName).
UWorld* World = GetWorld();
if (World == nullptr)
{
    return NAME_None;
}

ULootTierOverrideSubsystem* Subsystem = GetLootTierOverrideSubsystem(World);
if (Subsystem == nullptr)
{
    return NAME_None;
}

TArray<FLootTierOverrideEntry*> Overrides;
Subsystem->GetOverridesForBuildingClass(this, Overrides);

for (const FLootTierOverrideEntry* Entry : Overrides)
{
    if (Entry == nullptr || !Entry->OwnerClass->IsChildOf(ABuildingActor::StaticClass()))
    {
        continue;
    }
    for (const FLootTierOverrideMapping& Mapping : Entry->Mappings)
    {
        if (Mapping.BuildingTag == NAME_None)
        {
            continue;
        }
        if (StaticGameplayTags.GameplayTags.Contains(Mapping.BuildingTag))
        {
            return Mapping.LootTierGroup;
        }
    }
}

return NAME_None;
