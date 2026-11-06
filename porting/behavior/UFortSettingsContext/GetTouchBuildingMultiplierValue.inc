// CONFIDENCE: high
// Native 0x144d0d4f0; record offset 0x32c.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->TouchBuildingMultiplier : 0.0f;
