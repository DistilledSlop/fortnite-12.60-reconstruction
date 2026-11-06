// CONFIDENCE: high
// Native 0x144d0adb0 calls record GetShouldSprintByDefault.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->GetShouldSprintByDefault();
