// Original exec 0x142883a00; ordered unequal float compare and conditional dirty-byte store.
// CONFIDENCE: HIGH
const float PreviousValue = this->TouchBuildingMultiplier;
this->TouchBuildingMultiplier = InMultiplier;
if (PreviousValue < InMultiplier || PreviousValue > InMultiplier)
{
    this->bDirty = 1;
}
