// CONFIDENCE: HIGH. Native 0x141e3aa60; wrapper 0x14287f800.
if (InVolume < ChatVolume || InVolume > ChatVolume)
{
    ChatVolume = InVolume;
    bDirty = true;
}
