// CONFIDENCE: HIGH
// Original address: 0x142a71250 (exec thunk). Thunk read order: ChannelName (FString). It is passed by reference to both _Validate and _Implementation, then freed after the call.
// _Validate: vtable slot 0x2088 -> 0x1406801d0 = STUB (returns true).
// _Implementation: vtable slot 0x2090 -> 0x1423e9d60 (ported in AFortPlayerControllerZone__ServerVoiceChatRequestJoinToken_Implementation.inc).
if (!ServerVoiceChatRequestJoinToken_Validate(ChannelName))
{
	RPC_ValidateFailed(TEXT("ServerVoiceChatRequestJoinToken_Validate"));
}
else
{
	ServerVoiceChatRequestJoinToken_Implementation(ChannelName);
}
