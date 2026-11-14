// CONFIDENCE: HIGH. Original1426cfa80; water setting requires the exact integer value1.
const UFortWeaponRangedItemDefinition* RangedData = Cast<UFortWeaponRangedItemDefinition>(WeaponData);
if (NativeAllowFirstShotAccuracyInWater1478df440() != 1)
{
    const AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(GetInstigator());
    if (PlayerPawn && PlayerPawn->CurrentWaterBody)
    {
        return false;
    }
}
if (NativeHasAthenaInstigator1426d2410())
{
    return RangedData && RangedData->ShouldUsePerfectAimWhenTargetingMinSpread()
        && !UFortGlobals::IsFirstShotAccuracyDisabled();
}
return RangedData && RangedData->ShouldUsePerfectAimWhenTargetingMinSpread();
