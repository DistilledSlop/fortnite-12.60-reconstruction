// CONFIDENCE: high
// Native 0x144d011a0; reflected bool offset 0x470.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bAutoEquipBetterItems;
