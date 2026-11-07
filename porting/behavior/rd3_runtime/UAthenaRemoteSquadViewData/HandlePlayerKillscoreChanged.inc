// Behaviour snippet: UAthenaRemoteSquadViewData::HandlePlayerKillscoreChanged (body only; spliced into the generated member function).
// Original body: 0x144c280f0 (thunk 0x1451b9650). Offset evidence: PlayerDataArray is the TArray at +0x88 (Num at +0x90). Per entry: PlayerState (+0x1d0 on UAthenaRemotePlayerViewData) when non-null gives AFortPlayerStateAthena +0xe74 (int32), otherwise the entry's own +0x200 (int32). The total is compared with and stored to +0x9c (int32, NativeRecoveredInt_0x9c). OnSquadKillScoreChanged (+0x48, FIntDelegate) is broadcast with the packed {owner, total}; the owner pointer is this squad object, passed through reinterpret_cast to the delegate's PlayerData type. InPlayerData and InKillScore are not read by the asm.
// CONFIDENCE: MEDIUM
int32 TotalKillScore = 0;
for (int32 Index = 0; Index < PlayerDataArray.Num(); ++Index)
{
    UAthenaRemotePlayerViewData* PlayerData = PlayerDataArray[Index];
    if (PlayerData->PlayerState != nullptr)
    {
        TotalKillScore += PlayerData->PlayerState->NativeRecoveredInt32_0xe74;
    }
    else
    {
        TotalKillScore += PlayerData->NativeRecoveredInt32_0x200;
    }
}
if (NativeRecoveredInt_0x9c != TotalKillScore)
{
    NativeRecoveredInt_0x9c = TotalKillScore;
    OnSquadKillScoreChanged.Broadcast(reinterpret_cast<UAthenaRemotePlayerViewData*>(this), TotalKillScore);
}
