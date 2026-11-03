// CONFIDENCE: HIGH. Native 0x141e3b290; wrapper 0x14287fef0.
if (NewValue < EditButtonHoldTime || NewValue > EditButtonHoldTime)
{
    EditButtonHoldTime = NewValue;
    bDirty = true;
}
