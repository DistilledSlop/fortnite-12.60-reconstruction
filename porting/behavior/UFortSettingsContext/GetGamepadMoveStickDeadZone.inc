// CONFIDENCE: high
// Native 0x144d03f40; record offset 0x2dc.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->GamepadMoveStickDeadZone : 0.0f;
