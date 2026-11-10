// CONFIDENCE: LOW. The thunk reads four values; the reflection lists a separate UnderlyingType byte, folded into ModType here.
// The impl decompile drops the call arguments to FUN_1424b3340, so the stat-container argument is inferred from this+0x1a40.
if (StatsContainer != nullptr)   // this+0x1a40
{
    ModifyStatByName(StatsContainer, StatName, Amount, ModType);   // FUN_1424b3340; logs when the stat is not found
    if (bForceStatSave)
    {
        SaveStats(this);   // FUN_1423a9940 (217 lines, only partly read)
    }
}
