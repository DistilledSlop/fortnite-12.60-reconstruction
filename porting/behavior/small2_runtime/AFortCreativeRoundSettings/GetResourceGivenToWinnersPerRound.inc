// Behaviour snippet: AFortCreativeRoundSettings::GetResourceGivenToWinnersPerRound (body only; spliced into the generated member function).
// Original address: 0x141d84dd0
// Evidence: Count dword [RCX+0xc44] is unreflected: NativeRecoveredInt32_0xc44.
// Evidence: Element dword array at [RCX+0xc30+idx*4] (sign-extended int32 index) has no reflected member; raw access at object offset 0xc30.
// Evidence: Out-of-range index (signed, JS/JGE) returns 0.
// CONFIDENCE: MEDIUM
const int32 Index = static_cast<int32>(InResourceType);
const int32 Count = NativeRecoveredInt32_0xc44;
if (Index < 0 || Index >= Count)
{
    return 0;
}
const uint8* Raw = reinterpret_cast<const uint8*>(this);
return *reinterpret_cast<const int32*>(Raw + 0xc30 + static_cast<int64>(Index) * 4);
