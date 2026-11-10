// CONFIDENCE: MEDIUM
// Original address: 0x142a67450 (exec thunk); body is FUN_1423f2ef0 (0x1423f2ef0), ported inline here.
// Parameter ItemProvidingGhostMode is read from the FFrame as the single object parameter (class 0x142b56ce0).
// Authority-only ghost-mode entry. A pawn that already carries the ghost flag is left alone.
if (Role == ROLE_Authority && !GhostModeRepData.bInGhostMode)
{
    APawn* ControlledPawn = Pawn;
    if (ControlledPawn != nullptr
        && ControlledPawn->IsA(AFortPlayerPawnAthena::StaticClass())
        && (ControlledPawn->GhostFlagsWord_0x53a & 0x2) != 0)
    {
        return;
    }
    UnregisterGhostItemRef(ItemProvidingGhostMode);   // 0x141efa660: no effect in this build
    // The result of GetInterfaceAddress(UFortInventoryOwnerInterface) is discarded in this build.
    (void)GetInterfaceAddress(UFortInventoryOwnerInterface::StaticClass());
    // Stub_Empty (0x140680300): no effect.
    GhostModeRepData.bInGhostMode = true;
    GhostModeRepData.GhostModeItemDef = ItemProvidingGhostMode;
    UpdateGhostModeState();
    ForceNetUpdate();
}
