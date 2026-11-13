// CONFIDENCE: HIGH. Recovered dedicated-server branch; client lifecycle remains unresolved.
if (!UE_SERVER || GetNetMode() != NM_DedicatedServer)
{
    APlayerState::SetPlayerNameInternal(S);
    return;
}
// Dedicated-server branch of original 0x1421210a0.
// Preserve base assignment/dirty tracking and normal SetPlayerName virtual flow.
const FString WireName = IsOnlyASpectator() ? S : FortRecoveredPlayerName::Transform(S, 0, true);
APlayerState::SetPlayerNameInternal(WireName);
