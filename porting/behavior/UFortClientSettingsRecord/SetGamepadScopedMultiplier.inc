// Original exec 0x142880730; ordered unequal float compare and conditional dirty-byte store.
// CONFIDENCE: HIGH
const float PreviousValue = this->GamepadScopedSlow;
this->GamepadScopedSlow = InMultiplier;
if (PreviousValue < InMultiplier || PreviousValue > InMultiplier)
{
    this->bDirty = 1;
}
