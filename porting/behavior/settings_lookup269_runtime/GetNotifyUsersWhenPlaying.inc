// CONFIDENCE: high
// Native 0x144d067c0; reflected bool offset 0x744.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bNotifyUsersWhenPlaying;
