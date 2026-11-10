// CONFIDENCE: HIGH. Exec thunk 0x14292cd20 reads Event, EventSource, EventTarget and EventMultiplier, runs the _Validate virtual (vtable +0x958, Stub_ReturnTrue), reports failure through RPC_ValidateFailed, and otherwise calls the _Implementation virtual (vtable +0x960, 0x141eae750).
// Original address: 0x14292cd20 (exec).
if (!ServerFireAIDirectorEvent_Validate(Event, EventSource, EventTarget, EventMultiplier))
{
	RPC_ValidateFailed(TEXT("ServerFireAIDirectorEvent_Validate"));
	return;
}
ServerFireAIDirectorEvent_Implementation(Event, EventSource, EventTarget, EventMultiplier);
