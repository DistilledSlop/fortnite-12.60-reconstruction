// Native 0x1428804f0. CONFIDENCE: high
if (FMath::Abs(InSensitivity - GamepadLookSensitivity.X) > 1.e-8f)
{
    GamepadLookSensitivity.X = InSensitivity;
    bDirty = true;
}
