// Original exec 0x142881e40; ordered unequal float compare and conditional dirty-byte store.
// CONFIDENCE: HIGH
const float PreviousValue = this->MotionHarvestingToolSlow;
this->MotionHarvestingToolSlow = InMultiplier;
if (PreviousValue < InMultiplier || PreviousValue > InMultiplier)
{
    this->bDirty = 1;
}
