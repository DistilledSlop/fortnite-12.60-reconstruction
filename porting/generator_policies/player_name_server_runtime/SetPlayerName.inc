// CONFIDENCE: HIGH. Recovered dedicated-server branch; client lifecycle remains unresolved.
if (!UE_SERVER || GetNetMode() != NM_DedicatedServer)
{
    APlayerState::SetPlayerName(S);
    return;
}
// Original 0x142121070: +0x89c processing mask 2, NOT client-OnRep mask 4.
bNativePlayerNameSetterActive = true;
APlayerState::SetPlayerName(S);
bNativePlayerNameSetterActive = false;
