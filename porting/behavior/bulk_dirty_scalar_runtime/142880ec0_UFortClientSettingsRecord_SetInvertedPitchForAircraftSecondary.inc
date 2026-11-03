// Original exec 0x142880ec0, leaf 0x141e3bbe0; conditional Boolean and dirty-byte stores.
// CONFIDENCE: HIGH
if (this->bInvertedPitchForAircraftSecondary != bInvert)
{
    this->bInvertedPitchForAircraftSecondary = bInvert;
    this->bDirty = true;
}
