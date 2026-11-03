// Immediate integration contracts, requiring real definitions:
// UFortAssetManager& NativeFortAssetManager141df62a0();
// void NativeSetAssetManagerSubGame141deca00(UFortAssetManager&, ESubGame);
// UFortUIDirectorSubsystemBase* NativeFindUIDirectorSubsystem141856620(UFortGameInstance*);
// The subsystem declares virtual void NativeOnSubGameChanged298(const ESubGame&).
// The installed UFortUIDirector override calls NativeUpdateManager145176990();
// its parameter is ignored. NativeRefreshRegisteredPlayers141e78990 is the
// real GI operation, not a stub. The log category below aliases the actual
// category at 1481348e4; its source declaration/definition must be joined.

bool UFortGameInstance::NativeIsUIManagerChangeBlocked7bb() const
{
    return NativeUIManagerChangeBlocked7bb;
}

void UFortGameInstance::NativeSetSubGame141e7ec30(ESubGame NewSubGame)
{
    UE_LOG(LogNativeFortGameInstance1481348e4, VeryVerbose,
        TEXT("UFortGameInstance::SetCurrentSubGame - Called with SubGame %d, CurrentSubGame: %d"),
        static_cast<int32>(NewSubGame), static_cast<int32>(NativeSubGame828));
    if (NativeSubGame828 == NewSubGame)
    {
        return;
    }
    UE_LOG(LogNativeFortGameInstance1481348e4, VeryVerbose,
        TEXT("UFortGameInstance::SetCurrentSubGame - Changing SubGame to %d"),
        static_cast<int32>(NewSubGame));
    NativeSubGame828 = NewSubGame;
    NativeSetAssetManagerSubGame141deca00(NativeFortAssetManager141df62a0(), NewSubGame);
    if (UFortUIDirectorSubsystemBase* Director = NativeFindUIDirectorSubsystem141856620(this))
    {
        Director->NativeOnSubGameChanged298(NewSubGame);
    }
    NativeRefreshRegisteredPlayers141e78990(false);
    NativeSubGameChanged830.Broadcast(NewSubGame);
    OnSubGameChangedBP.Broadcast(NewSubGame);
}
