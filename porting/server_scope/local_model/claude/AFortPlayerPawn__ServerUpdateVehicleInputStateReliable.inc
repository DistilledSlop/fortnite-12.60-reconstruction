// CONFIDENCE: MEDIUM; thunk 0x142a7fb00: validate (vcall 0xfb8) is the return-true stub 0x1406801d0. Impl 0x14234ad20 is a tail-thunk into FUN_142a76100: it zero-initialises a 1-byte payload (FUN_141909530), stores the reliable input byte, looks up the UFunction by FName DAT_1481b4970 (name unresolved) and calls ProcessEvent with that payload.
FVehicleInputStateReliableParams Payload = {};
Payload.NativeRecovered_0x0 = ReliableInput.NativeRecovered_0x0;
ProcessEventVehicleInputStateReliable(Payload);
