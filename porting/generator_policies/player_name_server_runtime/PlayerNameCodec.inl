// Recovered dedicated-server path only. Generator policy must inject these
// definitions and corresponding declarations; do not edit generated native files.
// Original decode 0x142118d30; encode 0x142119770. See REPORT.md.
namespace FortRecoveredPlayerName
{
static FString Transform(const FString& Input, int8 Salt, bool bEncode)
{
#if UE_SERVER
    static_assert(sizeof(TCHAR) == sizeof(uint16), "Player name codec requires original UTF-16 TCHAR units");
    FString Result(Input);
    const int32 Count = Result.Len();
    uint32 State = (uint32(Count) + uint32(int32(Salt))) & 3u;
    for (int32 Index = 0; Index < Count; ++Index)
    {
        State += 3u;
        const uint16 Unit = uint16(Result[Index]);
        const uint16 Shift = uint16(State & 7u);
        Result[Index] = TCHAR(bEncode ? uint16(Unit - Shift) : uint16(Unit + Shift));
    }
    return Result;
#else
    return Input;
#endif
}
}
