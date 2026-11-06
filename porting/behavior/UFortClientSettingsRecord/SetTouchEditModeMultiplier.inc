// Original exec 0x142883b20; ordered unequal float compare and conditional dirty-byte store.
// CONFIDENCE: HIGH
const float PreviousValue = this->TouchEditModeMultiplier;
this->TouchEditModeMultiplier = InMultiplier;
if (PreviousValue < InMultiplier || PreviousValue > InMultiplier)
{
    this->bDirty = 1;
}
