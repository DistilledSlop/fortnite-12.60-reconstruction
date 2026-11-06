// Original exec 0x1428807c0; ordered unequal float compare and conditional dirty-byte store.
// CONFIDENCE: HIGH
const float PreviousValue = this->GamepadTargetingSlow;
this->GamepadTargetingSlow = InMultiplier;
if (PreviousValue < InMultiplier || PreviousValue > InMultiplier)
{
    this->bDirty = 1;
}
