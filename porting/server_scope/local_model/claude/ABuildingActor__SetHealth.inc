// CONFIDENCE: HIGH. Writes the building's Health attribute base value through the owning ability system component.
// Original address: 0x142738290 (exec). Native body: 0x141bc4a90.
static const FGameplayAttribute HealthAttribute(UFortBuildingActorSet::StaticClass()->FindPropertyByName(FName(TEXT("Health"))));

UFortBuildingActorSet* AttributeSet = GetBuildingAttributeSet();
UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentForAttributeSet(AttributeSet);
if (AbilitySystem)
{
    AbilitySystem->SetNumericAttributeBase(HealthAttribute, NewHealth);
}
