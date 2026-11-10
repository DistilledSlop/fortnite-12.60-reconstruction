// CONFIDENCE: LOW
// Original address: 0x142a5b6d0 (RPC thunk). FFrame reads: object (+0, 8 bytes), then FString (+8, 16 bytes).
// The thunk passes the object in the first argument slot and &FString in the second; the decompiled
// _Validate/_Implementation headers use (FString*, UObject*). Body semantics follow the headers.
if (!ServerLoadPlotForPortalFromMnemonic_Validate(Mnemonic, Portal))
{
	RPC_ValidateFailed(TEXT("ServerLoadPlotForPortalFromMnemonic_Validate"));
}
else
{
	ServerLoadPlotForPortalFromMnemonic_Implementation(Mnemonic, Portal);
}
if (Mnemonic.GetData() != nullptr)	// FString storage release (FMemory_Free) after the call
{
	FMemory_Free(Mnemonic.GetData());
}
