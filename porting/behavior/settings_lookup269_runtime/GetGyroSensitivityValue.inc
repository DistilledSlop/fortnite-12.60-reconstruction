// CONFIDENCE: high
// Native 0x144d040a0; record offset 0x33c.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->GyroSensitivity : 0.0f;
