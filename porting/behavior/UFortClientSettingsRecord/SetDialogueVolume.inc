// CONFIDENCE: HIGH. Native 0x141e3b240; wrapper 0x14287fde0.
if (InVolume < DialogVolume || InVolume > DialogVolume)
{
    DialogVolume = InVolume;
    bDirty = true;
}
