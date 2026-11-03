// Original exec 0x142883c40; ordered unequal float compare and conditional dirty-byte store.
// CONFIDENCE: HIGH
const float PreviousValue = this->TouchLookAccelerationMultiplier;
this->TouchLookAccelerationMultiplier = InSetting;
if (PreviousValue < InSetting || PreviousValue > InSetting)
{
    this->bDirty = 1;
}
