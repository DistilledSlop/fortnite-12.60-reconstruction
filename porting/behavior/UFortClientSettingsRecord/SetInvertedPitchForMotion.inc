// Original exec 0x142880f50, leaf 0x141e3bc00; conditional Boolean and dirty-byte stores.
// CONFIDENCE: HIGH
if (this->bInvertedPitchForMotion != bInvert)
{
    this->bInvertedPitchForMotion = bInvert;
    this->bDirty = true;
}
