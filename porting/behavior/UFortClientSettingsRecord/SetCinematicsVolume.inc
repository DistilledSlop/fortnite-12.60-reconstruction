// CONFIDENCE: HIGH. Native 0x141e3aa80; wrapper 0x14287f880.
if (InVolume < CinematicsVolume || InVolume > CinematicsVolume)
{
    CinematicsVolume = InVolume;
    bDirty = true;
}
