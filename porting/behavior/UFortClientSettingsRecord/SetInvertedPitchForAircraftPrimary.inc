// Original exec 0x142880e30, leaf 0x141e3bbc0; conditional Boolean and dirty-byte stores.
// CONFIDENCE: HIGH
if (this->bInvertedPitchForAircraftPrimary != bInvert)
{
    this->bInvertedPitchForAircraftPrimary = bInvert;
    this->bDirty = true;
}
