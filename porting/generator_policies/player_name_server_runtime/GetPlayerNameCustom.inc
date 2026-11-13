// CONFIDENCE: HIGH. Recovered dedicated-server branch; client lifecycle remains unresolved.
if (!UE_SERVER || GetNetMode() != NM_DedicatedServer)
{
    return APlayerState::GetPlayerNameCustom();
}
// Original 0x14211a390; proven dedicated-server branch.
const FString WireName = APlayerState::GetPlayerNameCustom();
if (bNativePlayerNameOnRepActive || IsOnlyASpectator())
{
    return WireName;
}
const FString PlainName = FortRecoveredPlayerName::Transform(WireName, 0, false);
// Base SetPlayerName caches this return value in OldNamePrivate. The original
// processing-bit2 branch stores that cache using independent salt 1.
return bNativePlayerNameSetterActive
    ? FortRecoveredPlayerName::Transform(PlainName, 1, true)
    : PlainName;
