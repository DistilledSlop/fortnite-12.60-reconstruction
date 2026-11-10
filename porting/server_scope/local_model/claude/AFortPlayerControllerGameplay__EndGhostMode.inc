// CONFIDENCE: MEDIUM
// Original address: 0x142a64fc0 (exec thunk, no parameters, no return value)
// Authority-only ghost-mode exit. Clears GhostModeRepData.bInGhostMode, stamps TimeExitedGhostMode from the world,
// refreshes ghost state through the shared tail (UpdateGhostModeState, 0x1423e0070), then forces a net update.
// The autonomous-proxy branch of the original is unreachable here: the entry check already requires ROLE_Authority.
if (Role != ROLE_Authority)
{
    return;
}
if (GhostModeRepData.GhostModeItemDef != nullptr)
{
    UnregisterGhostItemRef(GhostModeRepData.GhostModeItemDef);   // 0x141efa660: no effect in this build (always returns 0)
}
GhostModeRepData.bInGhostMode = false;
UWorld* World = GetWorld();
// Raw 32-bit copy of UWorld +0x540; the field is not resolved by name in the reflection dump (presumed world TimeSeconds).
GhostModeRepData.TimeExitedGhostMode = World->TimeSeconds;
UpdateGhostModeState();
ForceNetUpdate();
