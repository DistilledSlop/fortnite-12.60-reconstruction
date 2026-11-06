// Native 0x142882080. CONFIDENCE: high
if (FMath::Abs(InSensitivityMultiplier - MouseSensitivityMultiplierForAircraftValue) > 1.e-8f)
{
    MouseSensitivityMultiplierForAircraftValue = InSensitivityMultiplier;
    bDirty = true;
}
