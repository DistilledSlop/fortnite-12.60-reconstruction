// Original body address: 0x141b4d210 (thunk 0x14282e7a0)
// Offset evidence: this+0x270 = TeamLeadersInOrder (TArray<AFortPlayerStateAthena*>; Data at +0x270, Num at +0x278 read as MOVSXD).
// The loop compares each 8-byte element with RDX (Player); returns true on a match and false when none matches or the array is empty.
// CONFIDENCE: HIGH
for (int32 i = 0; i < TeamLeadersInOrder.Num(); ++i)
{
    if (TeamLeadersInOrder[i] == Player)
    {
        return true;
    }
}
return false;
