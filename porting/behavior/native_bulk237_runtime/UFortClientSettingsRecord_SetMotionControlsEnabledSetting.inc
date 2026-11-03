// Native 0x142881db0. CONFIDENCE: high
const bool bPreviousValue = bMotionControlsEnabled;
bMotionControlsEnabled = InSetting;
if (bPreviousValue != InSetting)
{
    bDirty = true;
}
