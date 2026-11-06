// CONFIDENCE: high
// Native 0x144d0d630.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->TouchVerticalMultiplier : 0.0f;
