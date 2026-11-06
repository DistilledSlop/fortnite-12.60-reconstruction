// CONFIDENCE: HIGH. Native 0x141e3c550; wrapper 0x1428827f0.
if (NewValue < RudderDeadZone || NewValue > RudderDeadZone)
{
    RudderDeadZone = NewValue;
    bDirty = true;
}
