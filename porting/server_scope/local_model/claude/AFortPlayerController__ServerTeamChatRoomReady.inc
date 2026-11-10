// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4dd50. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423ad070.
// Single FString parameter.
if (!ServerTeamChatRoomReady_Validate(ChatRoomId))
{
	RPC_ValidateFailed(TEXT("ServerTeamChatRoomReady_Validate"));
	return;
}
ServerTeamChatRoomReady_Implementation(ChatRoomId);
