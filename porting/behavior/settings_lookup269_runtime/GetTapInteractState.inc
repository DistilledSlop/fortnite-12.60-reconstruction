// CONFIDENCE: high
// Native 0x144d0b0a0; reflected bool offset 0x46c.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bTapInteractEnabled;
