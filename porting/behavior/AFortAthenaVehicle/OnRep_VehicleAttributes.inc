// CONFIDENCE: high; captured native branches, fields and engine contracts.
// Native 0x14283cab0 applies each replicated base value on non-authority vehicles.
if (GetLocalRole() != ROLE_Authority && VehicleMovementSet && AbilitySystemComponent)
{
    auto ApplyReplicatedBase = [this](FProperty* Property, float Value)
    {
        const FGameplayAttribute Attribute(Property);
        if (IAbilitySystemInterface* OwnerInterface = Cast<IAbilitySystemInterface>(VehicleMovementSet->GetOuter()))
        {
            if (UAbilitySystemComponent* OwnerAbilitySystem = OwnerInterface->GetAbilitySystemComponent())
            {
                OwnerAbilitySystem->SetNumericAttributeBase(Attribute, Value);
            }
        }
    };
    UClass* MovementSetClass = UFortVehicleMovementSet::StaticClass();
    static FProperty* FrontLateralFrictionScaleProperty = FindFProperty<FProperty>(MovementSetClass, TEXT("FrontLateralFrictionScale"));
    ApplyReplicatedBase(FrontLateralFrictionScaleProperty, VehicleAttributes.FrontLateralFrictionScale);
    static FProperty* RearLateralFrictionScaleProperty = FindFProperty<FProperty>(MovementSetClass, TEXT("RearLateralFrictionScale"));
    ApplyReplicatedBase(RearLateralFrictionScaleProperty, VehicleAttributes.RearLateralFrictionScale);
    static FProperty* ForwardForceTractionScaleProperty = FindFProperty<FProperty>(MovementSetClass, TEXT("ForwardForceTractionScale"));
    ApplyReplicatedBase(ForwardForceTractionScaleProperty, VehicleAttributes.ForwardForceTractionScale);
    static FProperty* BrakeForceTractionScaleProperty = FindFProperty<FProperty>(MovementSetClass, TEXT("BrakeForceTractionScale"));
    ApplyReplicatedBase(BrakeForceTractionScaleProperty, VehicleAttributes.BrakeForceTractionScale);
    static FProperty* SlopeAntigravityScaleProperty = FindFProperty<FProperty>(MovementSetClass, TEXT("SlopeAntigravityScale"));
    ApplyReplicatedBase(SlopeAntigravityScaleProperty, VehicleAttributes.SlopeAntigravityScale);
    static FProperty* TopSpeedScaleProperty = FindFProperty<FProperty>(MovementSetClass, TEXT("TopSpeedScale"));
    ApplyReplicatedBase(TopSpeedScaleProperty, VehicleAttributes.TopSpeedScale);
    static FProperty* VehicleGravityScaleProperty = FindFProperty<FProperty>(MovementSetClass, TEXT("VehicleGravityScale"));
    ApplyReplicatedBase(VehicleGravityScaleProperty, VehicleAttributes.VehicleGravityScale);
}
