// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142755ed0 calls ABuildingSMActor virtual +0xd80; real body 0x141c05a90.
// Only the authority applies the forced visual state (Role == ROLE_Authority, role byte 0xf0).
if (Role != ROLE_Authority)
{
    return;
}
// Dormant buildings are woken first (FUN_143f72040, AActor::FlushNetDormancy by its stat string) when NetDormancy is above DORM_Awake.
if (NetDormancy > DORM_Awake)
{
    FlushNetDormancy();
}
// Ensure the attribute set exists (FUN_141b9b750; lazy create of BuildingAttributeSet, result unused here).
EnsureBuildingAttributeSet();
// Apply the forced state through the timer path (FUN_141c1a5f0, receives the enum byte).
ScheduleForcedVisualState(VisualState);
