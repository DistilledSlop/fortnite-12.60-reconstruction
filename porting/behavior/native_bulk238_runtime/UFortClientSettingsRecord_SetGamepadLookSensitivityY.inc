// Native 0x142880590. CONFIDENCE: high
if (FMath::Abs(InSensitivity - GamepadLookSensitivity.Y) > 1.e-8f)
{
    GamepadLookSensitivity.Y = InSensitivity;
    bDirty = true;
}
