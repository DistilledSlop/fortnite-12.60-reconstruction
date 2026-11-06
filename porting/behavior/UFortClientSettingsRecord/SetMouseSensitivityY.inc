// Native 0x1428821c0. CONFIDENCE: high
if (FMath::Abs(InSensitivityY - MouseSensitivityY) > 1.e-8f)
{
    MouseSensitivityY = InSensitivityY;
    bDirty = true;
}
