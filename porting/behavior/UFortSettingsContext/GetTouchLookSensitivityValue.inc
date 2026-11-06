// CONFIDENCE: high
// Native 0x144d0d5d0; record offset 0x320.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->TouchLookSensitivitySetting : 0.0f;
