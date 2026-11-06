// CONFIDENCE: high
// Native 0x144d03fa0; record offset 0x2ec.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->GamepadTargetingSlow : 0.0f;
