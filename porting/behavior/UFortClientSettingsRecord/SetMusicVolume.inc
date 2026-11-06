// CONFIDENCE: HIGH. Native 0x141e3c210; wrapper 0x142882260.
if (InVolume < MusicVolume || InVolume > MusicVolume)
{
    MusicVolume = InVolume;
    bDirty = true;
}
