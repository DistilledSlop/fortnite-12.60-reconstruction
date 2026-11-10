// CONFIDENCE: HIGH
// Original address: 0x142a5b890 (RPC thunk). No parameters.
if (!ServerNotifyOstrichShieldOvercharge_Validate())
{
	RPC_ValidateFailed(TEXT("ServerNotifyOstrichShieldOvercharge_Validate"));
	return;
}
ServerNotifyOstrichShieldOvercharge_Implementation();
