// Native 0x141e3a2a0. CONFIDENCE: high
if (AllowAudioInBackground != Value)
{
    AllowAudioInBackground = Value;
    bDirty = true;
    bAllowAudioInBackground = static_cast<uint8>(Value) == 2;
}
