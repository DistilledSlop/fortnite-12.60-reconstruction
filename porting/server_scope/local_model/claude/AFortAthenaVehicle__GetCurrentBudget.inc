// CONFIDENCE: MEDIUM. Original addresses: exec thunk 0x14283b3c0, helper 0x1419153c0.
// The helper reports the vehicle's native budget only while the global budget feature flag is set
// (DAT_1478c24d8). Otherwise it returns 0.
if (!GFortVehicleBudgetEnabled)
{
    return 0;
}
return CurrentBudget;   // int32 at offset 0x450, native-only member (not in reflection)
