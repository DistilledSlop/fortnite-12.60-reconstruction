// CONFIDENCE: high
// Native 0x144d03f10; record offset 0x2e0.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->GamepadLookStickDeadZone : 0.0f;
