// CONFIDENCE: MEDIUM
// Original: 0x1423ad070 (virtual Implementation of ServerTeamChatRoomReady). Callees not named here are described in the evidence file.
// Forward the team chat room readiness to the game mode.
UE_LOG(LogFort, Log, TEXT("ServerTeamChatRoomReady_Implementation %s"), *ChatRoomId);
AFortGameMode* GameMode = Cast<AFortGameMode>(GetWorld()->AuthorityGameMode);
if (GameMode)
{
	GameMode->NativeHandleTeamChatRoomReady(this, ChatRoomId);
}
