// Original address: 0x1420e66c0
// CONFIDENCE: high
NativeResetFallingStartedZ1420edbc0();
const float MinZ = SurfaceSwimmingWaterControlParams.MaxSpeedUp.GetValueAtLevel(0.0f);
const float MaxDown = SurfaceSwimmingWaterControlParams.MaxSpeedDown.GetValueAtLevel(0.0f);
Velocity.Z = FMath::Clamp(Velocity.Z, -FMath::Abs(MaxDown), FMath::Abs(MinZ));
const float MaxEntryHoriz = SurfaceSwimmingWaterControlParams.MaxHorizontalEntrySpeed.GetValueAtLevel(0.0f);
const float AbsMaxEntry = FMath::Abs(MaxEntryHoriz);
if (AbsMaxEntry >= 0.0001f)
{
    FVector2D Horiz(Velocity.X, Velocity.Y);
    const float HorizSq = Horiz.SizeSquared();
    if (HorizSq > AbsMaxEntry * AbsMaxEntry)
    {
        Horiz = Horiz.GetSafeNormal() * AbsMaxEntry;
        Velocity.X = Horiz.X;
        Velocity.Y = Horiz.Y;
    }
}
else
{
    Velocity.X = 0.0f;
    Velocity.Y = 0.0f;
}
if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
{
    if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
    {
        const FGameplayTag& SwimTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x15e8);
        if (!ASC->HasMatchingGameplayTag(SwimTag))
        {
            ASC->AddLooseGameplayTag(SwimTag);
        }
    }
}
