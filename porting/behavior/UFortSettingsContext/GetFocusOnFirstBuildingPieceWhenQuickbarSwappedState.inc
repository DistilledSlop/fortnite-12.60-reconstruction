// CONFIDENCE: high
// Native 0x144d03cc0; reflected bool offset 0x480.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bNewFocusOnFirstBuildingPieceWhenQuickbarSwapped;
