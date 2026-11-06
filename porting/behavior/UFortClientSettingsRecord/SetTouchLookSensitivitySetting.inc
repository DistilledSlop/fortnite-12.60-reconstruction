// Original exec 0x142883cd0; ordered unequal float compare and conditional dirty-byte store.
// CONFIDENCE: HIGH
const float PreviousValue = this->TouchLookSensitivitySetting;
this->TouchLookSensitivitySetting = InSetting;
if (PreviousValue < InSetting || PreviousValue > InSetting)
{
    this->bDirty = 1;
}
