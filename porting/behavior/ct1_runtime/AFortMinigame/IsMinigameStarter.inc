// Body 0x14209a310 (thunk 0x1429c3290). Offset evidence: [this+0xaa8] is MinigameStarter (AFortPlayerState*). Checks in asm order: starter null, starter PendingKill, controller null, controller PendingKill (all false), then UFortMinigameStatics::NativeHelper142414c60(MinigameStarter) compared with the controller pointer.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortPlayerState) > 0, "AFortPlayerState must be complete for MinigameStarter->IsPendingKill()");
static_assert(sizeof(AFortPlayerController) > 0, "AFortPlayerController must be complete for PlayerController->IsPendingKill()");
static_assert(sizeof(UFortMinigameStatics) > 0, "UFortMinigameStatics must be declared to call the ported helper");
if (MinigameStarter == nullptr)
{
    return false;
}
if (MinigameStarter->IsPendingKill())
{
    return false;
}
if (PlayerController == nullptr)
{
    return false;
}
if (PlayerController->IsPendingKill())
{
    return false;
}
return reinterpret_cast<uint8*>(UFortMinigameStatics::NativeHelper142414c60(MinigameStarter)) == reinterpret_cast<uint8*>(PlayerController);
