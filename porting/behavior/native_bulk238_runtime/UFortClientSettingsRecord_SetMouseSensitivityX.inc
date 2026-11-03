// Native 0x142882120. CONFIDENCE: high
if (FMath::Abs(InSensitivityX - MouseSensitivityX) > 1.e-8f)
{
    MouseSensitivityX = InSensitivityX;
    bDirty = true;
}
