// CONFIDENCE: HIGH
// Original 0x1418a1010.
if (!Definition)
{
    UE_LOG(LogAthenaBots, Error, TEXT("UFortAthenaAIBotInventoryDigestedSkillSet::GetInfiniteAmmoCheatEnabled, invalid item definition"));
    return false;
}
FGameplayTagContainer DefinitionTags;
Definition->GetOwnedGameplayTags(DefinitionTags);
return DefinitionTags.HasAnyExact(InfiniteAmmoCheats);
