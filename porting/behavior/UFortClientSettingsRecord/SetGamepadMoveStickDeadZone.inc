// CONFIDENCE: HIGH. Native 0x141e3b6b0; wrapper 0x1428806b0.
if (FMath::Abs(NewValue - GamepadMoveStickDeadZone) > 1.e-8f)
{
    GamepadMoveStickDeadZone = NewValue;
    bDirty = true;
}
