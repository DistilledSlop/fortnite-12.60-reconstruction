// CONFIDENCE: high
// Native 0x144d0b0d0; record offset 0x2e4.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->TargetingSlow : 0.0f;
