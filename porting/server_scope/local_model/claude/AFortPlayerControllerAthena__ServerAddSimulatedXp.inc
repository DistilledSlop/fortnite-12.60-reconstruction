// CONFIDENCE: HIGH
// Original address: 0x142a5a6e0 (RPC thunk). FFrame read order: XpAmmount (float), Source (FText), MetaData (FGameplayTag).
// _Validate target 0x1406801d0 is Stub_ReturnTrue.
if (!ServerAddSimulatedXp_Validate(XpAmmount, Source, MetaData))
{
	RPC_ValidateFailed(TEXT("ServerAddSimulatedXp_Validate"));
	return;
}
ServerAddSimulatedXp_Implementation(XpAmmount, Source, MetaData);
