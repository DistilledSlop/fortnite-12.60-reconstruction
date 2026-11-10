// CONFIDENCE: HIGH. Authority-only. Ensures the building attribute set exists, then writes MaxHealth and Health on the owning ability system component, both set to NewHealth.
// Original address: 0x142736690 (exec). Native body: 0x141b990d0 (inlines the same attribute-set creation as GetBuildingAttributeSet).
if (Role != ROLE_Authority)
{
    return;
}

static const FGameplayAttribute MaxHealthAttribute(UFortBuildingActorSet::StaticClass()->FindPropertyByName(FName(TEXT("MaxHealth"))));
static const FGameplayAttribute HealthAttribute(UFortBuildingActorSet::StaticClass()->FindPropertyByName(FName(TEXT("Health"))));

UFortBuildingActorSet* AttributeSet = GetBuildingAttributeSet();
UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentForAttributeSet(AttributeSet);
if (AbilitySystem)
{
    AbilitySystem->SetNumericAttributeBase(MaxHealthAttribute, NewHealth);
    AbilitySystem->SetNumericAttributeBase(HealthAttribute, NewHealth);
}
