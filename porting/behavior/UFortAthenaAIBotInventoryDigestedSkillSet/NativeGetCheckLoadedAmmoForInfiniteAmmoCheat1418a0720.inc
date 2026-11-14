// CONFIDENCE: HIGH
// Original 0x1418a0720.
if (!Definition)
{
    UE_LOG(LogAthenaBots, Error, TEXT("UFortAthenaAIBotInventoryDigestedSkillSet::GetCheckLoadedAmmoForInfiniteAmmoCheat, invalid item definition"));
    return true;
}
FGameplayTagContainer DefinitionTags;
Definition->GetOwnedGameplayTags(DefinitionTags);
return DefinitionTags.HasAnyExact(CheckLoadedAmmoForInfiniteAmmoCheats);
