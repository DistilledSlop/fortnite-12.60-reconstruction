// CONFIDENCE: MEDIUM - control flow fully read; the event FName (DAT_1481aef78) is not resolved to a string.
// Original address: 0x142a46a60 (exec thunk, no parameters, no return value).
// Role is AActor::Role at offset 0xF0 (the decompile reads it as param_1[6]). ROLE_Authority == 3.
// The only side effect is one ProcessEvent into a Blueprint event with a null parameter block.
if (Role != ENetRole::ROLE_Authority)
{
	return;
}
UFunction* Event = FindFunctionChecked(FName_0x1481aef78);
ProcessEvent(Event, nullptr);
