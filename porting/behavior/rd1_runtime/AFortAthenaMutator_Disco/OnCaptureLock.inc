// Original body address: 0x141adc620 (thunk 0x1427fe6c0). Body asm: 38 instructions.
// Offset evidence: [RCX+0x448] = SpawnedControlPoints data, [RCX+0x450] = Num (reflected Array), element stride 0x50 with qword +0x0 = point pointer (raw).
// Point pointee (raw): +0x9f8 bit 0, +0xaa8 bit 0, +0xab9 byte compared with LockTeam.
// [RCX+0x260] = CachedGameState (reflected, inherited AFortAthenaMutator). Pointee writes (raw int32): +0x1f00 phase, +0x1f04 lock team, +0x1f08 counter increment.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const int32 NumPoints = SpawnedControlPoints.Num();
const uint8* Points = *reinterpret_cast<const uint8* const*>(Raw + 0x448);
int32 CountA = 0;
int32 CountB = 0;
for (int32 i = 0; i < NumPoints; ++i)
{
    const uint8* Point = *reinterpret_cast<const uint8* const*>(Points + static_cast<int64>(i) * 0x50);
    if (Point != nullptr && (Point[0x9f8] & 0x1) != 0)
    {
        ++CountA;
        if ((Point[0xaa8] & 0x1) != 0 && Point[0xab9] == LockTeam)
        {
            ++CountB;
        }
    }
}
uint8* GameState = reinterpret_cast<uint8*>(CachedGameState);
const int32 NewState = (CountA > 1 && CountB >= CountA) ? 2 : 3;
*reinterpret_cast<int32*>(GameState + 0x1f00) = NewState;
*reinterpret_cast<int32*>(GameState + 0x1f04) = static_cast<int32>(LockTeam);
*reinterpret_cast<int32*>(GameState + 0x1f08) += 1;
