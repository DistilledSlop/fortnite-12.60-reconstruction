// CONFIDENCE: HIGH. Native 0x141e3c570; wrapper 0x142882870.
if (NewValue < RudderMaxThrottle || NewValue > RudderMaxThrottle)
{
    RudderMaxThrottle = NewValue;
    bDirty = true;
}
