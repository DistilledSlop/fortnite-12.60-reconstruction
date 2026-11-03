// CONFIDENCE: high
// Native 0x144d0d600.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->TouchTargetingSlow : 0.0f;
