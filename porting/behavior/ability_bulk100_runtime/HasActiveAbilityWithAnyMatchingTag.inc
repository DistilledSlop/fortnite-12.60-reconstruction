// CONFIDENCE: high
// Native 0x1416011b0, reflected wrapper 0x142781560.
for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
{
    if (Spec.IsActive() && Spec.Ability->AbilityTags.HasAny(Tags))
    {
        return true;
    }
}
return false;
