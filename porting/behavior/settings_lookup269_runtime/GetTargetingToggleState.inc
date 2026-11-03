// CONFIDENCE: high
// Native 0x144d0b100 calls record GetTargetingToggleable.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->GetTargetingToggleable();
