// CONFIDENCE: HIGH. Recovered dedicated-server branch; client lifecycle remains unresolved.
if (!UE_SERVER || GetNetMode() != NM_DedicatedServer)
{
    return APlayerState::GetOldPlayerName();
}
// Original 0x14211a290. The original inherited SetOldPlayerName is raw assignment.
const FString WireOldName = APlayerState::GetOldPlayerName();
return IsOnlyASpectator() ? WireOldName : FortRecoveredPlayerName::Transform(WireOldName, 1, false);
