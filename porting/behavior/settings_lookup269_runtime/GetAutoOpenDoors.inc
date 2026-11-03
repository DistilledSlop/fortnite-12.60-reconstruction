// CONFIDENCE: high
// Native 0x144d01370 calls record GetAutoOpenDoors.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->GetAutoOpenDoors();
