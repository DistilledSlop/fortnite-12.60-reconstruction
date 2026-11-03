// CONFIDENCE: high
// Native 0x144d03e20; record offset 0x2f4.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->GamepadBuildingMultiplier : 0.0f;
