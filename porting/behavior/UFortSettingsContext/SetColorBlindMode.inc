// CONFIDENCE: high
// Native 0x144d1d940.
if (UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50())
{
    Record->SetColorBlindMode(static_cast<EColorBlindMode>(static_cast<uint8>(InMode)));
}
