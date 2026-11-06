// Original exec 0x142881070, leaf 0x141e3bc70; conditional Boolean and dirty-byte stores.
// CONFIDENCE: HIGH
if (this->bInvertedYawForMotion != bInvert)
{
    this->bInvertedYawForMotion = bInvert;
    this->bDirty = true;
}
