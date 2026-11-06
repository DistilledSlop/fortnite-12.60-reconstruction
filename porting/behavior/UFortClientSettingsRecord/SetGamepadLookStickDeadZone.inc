// CONFIDENCE: HIGH. Native 0x141e3b680; wrapper 0x142880630.
if (FMath::Abs(NewValue - GamepadLookStickDeadZone) > 1.e-8f)
{
    GamepadLookStickDeadZone = NewValue;
    bDirty = true;
}
