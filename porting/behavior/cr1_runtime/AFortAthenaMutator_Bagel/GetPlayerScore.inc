// Original body address: 0x141aae6e0
// Offset evidence: object+0x12b0 (Data, uint64) and object+0x12b8 (Num, int32) are unreflected in the own region and are recovered as typed members (NativeRecoveredUint64_0x12b0, NativeRecoveredInt32_0x12b8). Records are 0x80 bytes (SHL RSI,7). Record+0x10 (qword) is compared with AthenaPC->PlayerState when that is an AFortPlayerStateAthena (else 0); on a match OutScore = Record+0x50 (int32) and the result is true. AthenaPC+0x220 = AController::PlayerState (reflected).
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortPlayerControllerAthena) > 0, "AFortPlayerControllerAthena must be complete for PlayerState access");
static_assert(sizeof(AFortPlayerStateAthena) > 0, "AFortPlayerStateAthena must be complete for its StaticClass check");
if (AthenaPC == nullptr)
{
    return false;
}
const uint8* RecordBegin = reinterpret_cast<const uint8*>(NativeRecoveredUint64_0x12b0);
const uint8* RecordEnd = RecordBegin + (static_cast<int64>(NativeRecoveredInt32_0x12b8) << 7);
if (RecordBegin == RecordEnd)
{
    return false;
}
const APlayerState* WantedPlayerState = AthenaPC->PlayerState;
const uint64 WantedKey = (WantedPlayerState != nullptr && WantedPlayerState->IsA(AFortPlayerStateAthena::StaticClass()))
    ? reinterpret_cast<uint64>(WantedPlayerState)
    : 0;
const uint8* Record = RecordBegin;
do
{
    if (*reinterpret_cast<const uint64*>(Record + 0x10) == WantedKey)
    {
        OutScore = *reinterpret_cast<const int32*>(Record + 0x50);
        return true;
    }
    Record += 0x80;
} while (Record != RecordEnd);
return false;
