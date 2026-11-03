// CONFIDENCE: high
// Native 0x144d07020 calls record GetInvertedPitch.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->GetInvertedPitch();
