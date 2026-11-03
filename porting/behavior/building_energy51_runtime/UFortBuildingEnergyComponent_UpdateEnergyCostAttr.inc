// CONFIDENCE: HIGH, subject to the interface restoration in HANDOFF.md.
if (EnergyComponentAttrSet)
{
    static FProperty* AttributeProperty = FindFieldChecked<FProperty>(
        UFortBuildingEnergyComponentAttrSet::StaticClass(), FName(TEXT("EnergyCost")));
    const FGameplayAttribute Attribute(AttributeProperty);
    if (ABuildingActor* Building = Cast<ABuildingActor>(GetOwner()))
    {
        if (UFortAbilitySystemComponent* AbilityComponent = Cast<UFortAbilitySystemComponent>(
                static_cast<IAbilitySystemInterface*>(Building)->GetAbilitySystemComponent()))
        {
            AbilityComponent->ApplyModToAttribute(Attribute, EGameplayModOp::Override, NewEnergyCost);
        }
    }
}
