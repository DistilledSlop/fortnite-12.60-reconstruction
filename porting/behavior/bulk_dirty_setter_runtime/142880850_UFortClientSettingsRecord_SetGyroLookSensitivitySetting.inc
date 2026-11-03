// Original exec 0x142880850; ordered unequal float compare and conditional dirty-byte store.
// CONFIDENCE: HIGH
const float PreviousValue = this->GyroSensitivity;
this->GyroSensitivity = InSetting;
if (PreviousValue < InSetting || PreviousValue > InSetting)
{
    this->bDirty = 1;
}
