// CONFIDENCE: MEDIUM. Original addresses: exec 0x142a7e790 (calls thunk to 0x142a77460, read in full: FindFunctionChecked then ProcessEvent with one parameter).
// The event is a Blueprint-side function looked up by an FName constant (data 0x1481b4a98, not resolved: LOW). The instigator is passed by pointer.
UFunction* ReviveEvent = FindFunctionChecked(ReviveFromDBNOEventName);   // FName @0x1481b4a98 (LOW)
ProcessEvent(ReviveEvent, &EventInstigator);
