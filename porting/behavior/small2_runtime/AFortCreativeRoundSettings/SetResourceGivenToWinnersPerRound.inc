// Behaviour snippet: AFortCreativeRoundSettings::SetResourceGivenToWinnersPerRound (body only; spliced into the generated member function).
// Original address: 0x141d91030
// Evidence: Count dword [RCX+0xc44] is unreflected: NativeRecoveredInt32_0xc44.
// Evidence: Element dword store at [RCX+0xc30+idx*4]; value is Amount clamped to 0 when negative (CMOVNS on R8D).
// Evidence: Out-of-range index (signed) skips the store.
// CONFIDENCE: MEDIUM
const int32 Index = static_cast<int32>(InResourceType);
const int32 Count = NativeRecoveredInt32_0xc44;
if (Index < 0 || Index >= Count)
{
    return;
}
uint8* Raw = reinterpret_cast<uint8*>(this);
*reinterpret_cast<int32*>(Raw + 0xc30 + static_cast<int64>(Index) * 4) = Amount < 0 ? 0 : Amount;
