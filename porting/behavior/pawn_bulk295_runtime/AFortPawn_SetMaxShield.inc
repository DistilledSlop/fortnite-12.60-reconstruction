// CONFIDENCE: high; captured native behavior and reflected types.
// Native 0x14234dbc0 sets this attribute's base through the set owner's ability-system interface.
if (HealthSet)
{
    static FProperty* AttributeProperty = FindFProperty<FProperty>(UFortHealthSet::StaticClass(), FName(TEXT("Shield")));
    const FGameplayAttribute Attribute(AttributeProperty);
    if (IAbilitySystemInterface* OwnerInterface = Cast<IAbilitySystemInterface>(HealthSet->GetOuter()))
    {
        if (UAbilitySystemComponent* AbilityComponent = OwnerInterface->GetAbilitySystemComponent())
        {
            AbilityComponent->SetNumericAttributeBase(Attribute, NewValue);
        }
    }
}
