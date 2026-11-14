// Original address: 0x1420d9940
// CONFIDENCE: high
if (!NativeHasSurfaceWater1420e5490())
{
    return false;
}
if (MovementMode == MOVE_Custom && CustomMovementMode == 10)
{
    return false;
}
AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (!Player)
{
    return false;
}
const float TargetDepth = (Player->bIsDBNO ? SurfaceSwimmingWaterControlParams.DBNOSwimImmersionDepth : SurfaceSwimmingWaterControlParams.TargetImmersionDepth).GetValueAtLevel(0.0f);
if (NativeRecoveredFloat_0x1d08 > TargetDepth && Velocity.Z <= 0.0f)
{
    if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
    {
        const FGameplayTag& SwimTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x15e8);
        if (ASC->HasMatchingGameplayTag(SwimTag))
        {
            SetMovementMode(MOVE_Custom, 10);
            NativeResetFallingStartedZ1420edbc0();
            return true;
        }
    }
}
return false;
