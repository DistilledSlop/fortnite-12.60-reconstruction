// CONFIDENCE: LOW. Forgets the pending query and clears the hit bookkeeping (hit records, hit bits and the index table) when the record counters disagree.
// Original address: 0x14274be90 (exec, inline body).
HitActorsQueryHandle = 0;                            // +0xd58
if (HitRecordCount != HitRecordCapacity)             // +0xd10 vs +0xd3c
{
    HitRecordCount = 0;                              // +0xd10
    if (HitRecordsSlackFlag < 0)                     // +0xd14: the record array uses its inline storage
    {
        HitRecords.Reserve(0);                       // native 0x1406911b0 (16-byte elements)
    }
    HitRecordsFreeIndex = -1;                        // +0xd38
    HitRecordCapacity = 0;                           // +0xd3c
    const int32 Bits = HitFlagBitCount;              // +0xd30
    HitFlagBits.ClearBits((Bits + 31) / 32);        // +0xd18 or heap at +0xd28, zeroed in 32-bit words
    HitFlagBitCount = 0;
    if (HitIndexCount > 0)                           // +0xd50
    {
        HitIndexTable.FillWith(-1);                  // +0xd40 or heap at +0xd48, int32 entries set to -1
    }
}
