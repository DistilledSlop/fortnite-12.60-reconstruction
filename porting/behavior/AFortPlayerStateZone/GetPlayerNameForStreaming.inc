// CONFIDENCE: high
// Helper1444ac7a0 tests custom-player-name bit then calls custom name virtual or copies private FString; exact APlayerState::GetPlayerName behavior in UE4.25 PlayerState.cpp304. Preserve custom virtual dispatch and FString copy semantics.
return APlayerState::GetPlayerName();
