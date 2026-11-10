// CONFIDENCE: MEDIUM. Stores the big-health value and, while it stays below the building attribute set's max, writes it to the Health attribute through the ability system.
// Original address: 0x1427509e0 (exec). Native body: 0x141c1b670.
BigHealth = NewHealth;                                                       // int64 at +0xb98
if (UFortBuildingActorSet* AttributeSet = GetBuildingAttributeSet())         // native 0x141b9b7c0 (+0x318)
{
    const float MaxHealth = AttributeSet->MaxHealth;                         // native 0x14070cc00 (int at attribute set +0x3c)
    if (static_cast<float>(BigHealth) < MaxHealth)
    {
        const FGameplayAttribute HealthAttribute = GetHealthAttribute();     // native 0x141916570, cached UFortBuildingActorSet::Health
        if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentForAttributeSet(AttributeSet))   // native 0x14070f1a0
        {
            AbilitySystem->SetNumericAttributeBase(HealthAttribute, static_cast<float>(BigHealth));   // native 0x1406da850 via 0x141bc4d60
        }
    }
}
