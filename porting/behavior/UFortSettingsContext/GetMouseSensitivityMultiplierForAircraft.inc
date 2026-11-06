// CONFIDENCE: high
// Native 0x144d066d0; record offset 0x314.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->MouseSensitivityMultiplierForAircraftValue : 0.0f;
