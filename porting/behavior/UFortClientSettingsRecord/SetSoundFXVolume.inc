// CONFIDENCE: HIGH. Native 0x141e3c9a0; wrapper 0x142883140.
if (InVolume < SoundFXVolume || InVolume > SoundFXVolume)
{
    SoundFXVolume = InVolume;
    bDirty = true;
}
