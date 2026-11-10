// CONFIDENCE: MEDIUM. Original addresses: exec thunk 0x14283cf90, virtual slot 0x828 -> 0x1419205c0.
// The virtual target forwards to the Blueprint-implemented event: FindFunctionChecked(FName 0x3236d),
// then ProcessEvent with the one-byte FireModeNum parameter.
UFunction* ServerStartFireEvent = FindFunctionChecked(ServerStartFireEventName);   // FName index 0x3236d (DAT_148189348)
ProcessEvent(ServerStartFireEvent, &FireModeNum);
