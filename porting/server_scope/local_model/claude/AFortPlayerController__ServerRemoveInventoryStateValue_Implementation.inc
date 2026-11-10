// CONFIDENCE: LOW. Impl at 0x1423a8370 is fully read, but its decompile shows only two parameters.
// The StateValueType argument the thunk passes is not visible in the impl; FUN_142a3f270 is not read.
if (MatchPhase != 3)   // byte at this+0xf0
{
    RemoveInventoryStateValue(ItemGuid.First32, ItemGuid);   // FUN_142a3f270(first dword of guid, &guid); argument mapping unverified
}
