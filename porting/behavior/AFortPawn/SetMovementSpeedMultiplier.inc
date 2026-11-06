// CONFIDENCE: high; captured native behavior and reflected types.
// Native 0x14234df40 sets this attribute's base through the set owner's ability-system interface.
if (MovementSet)
{
    static FProperty* AttributeProperty = FindFProperty<FProperty>(UFortMovementSet::StaticClass(), FName(TEXT("SpeedMultiplier")));
    const FGameplayAttribute Attribute(AttributeProperty);
    if (IAbilitySystemInterface* OwnerInterface = Cast<IAbilitySystemInterface>(MovementSet->GetOuter()))
    {
        if (UAbilitySystemComponent* AbilityComponent = OwnerInterface->GetAbilitySystemComponent())
        {
            AbilityComponent->SetNumericAttributeBase(Attribute, NewMovementSpeedVal);
        }
    }
}
