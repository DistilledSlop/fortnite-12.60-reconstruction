// CONFIDENCE: LOW - the body is a ProcessEvent into one Blueprint event with a copy of ChoiceData. The event parameter block is only partly mapped (its leading int32 and byte, plus a copied string and an array copy); the Blueprint event FName (DAT_1481aefb8) is not resolved.
// Original address: 0x142a492a0 (exec thunk, FChoiceData parameter). Copy helper: 0x1423a0880. Event dispatch: 0x142a3bee0 (211 lines, partly decoded).
// The thunk copies ChoiceData into a local, then dispatches the event with that copy as the parameter block.
FChoiceData LocalChoiceData = ChoiceData;
UFunction* Event = FindFunctionChecked(FName_0x1481aefb8);
ProcessEvent(Event, &LocalChoiceData);
