// CONFIDENCE: HIGH
// Original address: 0x142a5a960 (RPC thunk). FFrame read order: GUID (16 bytes, +0), ItemWrap (soft object, +16).
if (!ServerApplyOverrideWrapToItem_Validate(GUID, ItemWrap))
{
	RPC_ValidateFailed(TEXT("ServerApplyOverrideWrapToItem_Validate"));
	return;
}
ServerApplyOverrideWrapToItem_Implementation(GUID, ItemWrap);
