// CONFIDENCE: high
// Native 0x1415ece70, reflected wrapper 0x142780be0; 0x14070f400 is GetPrimaryInstance.
TArray<FGameplayAbilitySpec*> MatchingAbilities;
GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayAbilityTags, MatchingAbilities, true);
if (MatchingAbilities.Num() == 0)
{
    return false;
}
for (const FGameplayAbilitySpec* Spec : MatchingAbilities)
{
    if (!Spec->GetPrimaryInstance()->CanActivateAbility(Spec->Handle, AbilityActorInfo.Get(), nullptr, nullptr, nullptr))
    {
        return false;
    }
}
return true;
