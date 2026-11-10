// CONFIDENCE: HIGH
// Original address: 0x142a5b7f0 (RPC thunk). No parameters.
if (!ServerNotifyClientReachedMaxWaitingTimeForDeviceReplication_Validate())
{
	RPC_ValidateFailed(TEXT("ServerNotifyClientReachedMaxWaitingTimeForDeviceReplication_Validate"));
	return;
}
ServerNotifyClientReachedMaxWaitingTimeForDeviceReplication_Implementation();
