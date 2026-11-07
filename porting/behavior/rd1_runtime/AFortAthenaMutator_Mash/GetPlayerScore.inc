// Original body address: 0x141aae6e0 (thunk 0x1427e8db0). Body asm: 48 instructions. Shared body with AFortAthenaMutator_Bagel::GetPlayerScore.
// Offset evidence: [RCX+0x12b0] = element data pointer, [RCX+0x12b8] = int32 Count, element stride 0x80: qword +0x10 is compared with the key, int32 +0x50 is the score. Span is unreflected (raw).
// [AthenaPC+0x220] = PlayerState (reflected AController::PlayerState). Key is PlayerState when it IsA AFortPlayerStateAthena, else null.
// CONFIDENCE: MEDIUM
if (AthenaPC == nullptr) { return false; }
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* Begin = *reinterpret_cast<const uint8* const*>(Raw + 0x12b0);
const int32 Count = *reinterpret_cast<const int32*>(Raw + 0x12b8);
const uint8* End = Begin + static_cast<int64>(Count) * 0x80;
const UObject* PlayerStateObject = AthenaPC->PlayerState;
const void* Key = nullptr;
if (PlayerStateObject != nullptr && PlayerStateObject->IsA(AFortPlayerStateAthena::StaticClass()))
{
    Key = PlayerStateObject;
}
for (const uint8* Element = Begin; Element != End; Element += 0x80)
{
    if (*reinterpret_cast<const void* const*>(Element + 0x10) == Key)
    {
        OutScore = *reinterpret_cast<const int32*>(Element + 0x50);
        return true;
    }
}
return false;
