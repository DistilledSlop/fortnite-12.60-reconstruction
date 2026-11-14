// CONFIDENCE: HIGH. Original1423304f0; style refresh and action reconciliation are unconditional.
const auto& NativeTags = FFortNativeGameplayTagsRuntime::Get();
if (bool(bIsTargeting) != bNewTargeting)
{
    bIsTargeting = bNewTargeting;
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetTagMapCount(NativeTags.GetTagAtOriginalOffset(0x638), bIsTargeting ? 1 : 0);
    }
    if (AFortAthenaVehicle* Vehicle = Cast<AFortAthenaVehicle>(VehicleStateLocal.Vehicle))
    {
        Vehicle->NativeOnTargetingChanged_d70(this, VehicleStateLocal.SeatIndex, bool(bIsTargeting));
    }
}
NativeUpdateMovementStyle14235bbf0();
const FGameplayTag& AimAction = NativeTags.GetTagAtOriginalOffset(0x768);
const bool bActive = HasNativeAction(AimAction);
if (bNewTargeting)
{
    if (!bActive)
    {
        NativeStartAction142353060(AimAction);
    }
}
else if (bActive)
{
    NativeStopAction142353960(AimAction);
}
