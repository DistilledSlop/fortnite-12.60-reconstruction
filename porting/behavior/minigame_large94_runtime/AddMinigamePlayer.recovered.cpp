// AFortMinigame::AddMinigamePlayer(AFortPlayerState* Player, bool bForced) -- native 0x142081fa0.
// ROUGH SOURCE, NOT CLOSED: every Minigame94_* symbol not defined in AddMinigamePlayer.bindings.inc is an unresolved
// native helper (listed there with its address). Bound pieces use reflected fields/engine calls only.
// Preserved from native: branch order, log gates (verbosity >2 invalid; >4 already-registered / added), array-dirty then
// subobject-state then SolPlayer order, weak-pointer player handed to setup, pawn broadcast after setup, controller
// back-pointer set on every path that reaches it with a non-null controller.
// Fixes vs the old proposal: team passed to assignment is PlayerBuckets[Index].TeamIdAtRoundStart; +0x39 test is
// FMinigamePlayer::ClassSlotIndex (not team); pawn "add" is OnPlayerJoinInProgress.Broadcast(Pawn); +0xaa5 is PlayerPersistence.
#include "AddMinigamePlayer.bindings.inc"

void AFortMinigame::AddMinigamePlayer(AFortPlayerState* Player, bool bForced)
{
    if (!Minigame94_IsAuthority(this)) { return; }
    if (!Player)
    {
        Minigame94_LogAddMinigamePlayer(/*Verbosity>2, level 3*/ 3, TEXT("%s.AddMinigamePlayer - Invalid player state."), this, nullptr);
        return;
    }

    const bool bManagerOverride = Minigame94_ManagerAllowsImmediateJoin(this); // Volume(+0x910) null => false
    const int32 Mode = Minigame94_Mode(this);
    const bool bPlaying = (uint32)(Mode - 2) < 2u || bForced || bManagerOverride;
    const uint8 PreferredTeam = (Mode == 3) ? Minigame94_PreferredTeamRaw(this) : 0xff;

    UObject* WorldContext = Minigame94_GetWorldContext(this);
    if (!WorldContext) { return; }

    const EFortMinigameState State = CurrentState;
    FMinigamePlayers& Array =
        (State == EFortMinigameState::PreGame || bPlaying || State == EFortMinigameState::PostGameReset)
            ? PlayerArray : SpectatingPlayerArray;

    FMinigamePlayer* Record = Minigame94_FindOrCreatePlayerRecord(Array, Player->GetUniqueId()); // Player+0x248
    if (!Record) { return; }

    APawn* Pawn = Minigame94_PlayerPawn(Player);
    AFortPlayerControllerAthena* Controller = Minigame94_AthenaController(Player);

    if (Minigame94_RejectLateJoinGlobalEnabled() && Controller && Minigame94_RejectLateJoin(this) &&
        (uint8)State > 5 && State != EFortMinigameState::PostGameReset)
    {
        Minigame94_SendControllerMessage(Controller, Minigame94_JoinTooLateText());
        return;
    }

    if (Minigame94_IsRegistered(*Record, WorldContext, TEXT("AFortMinigame::AddMinigamePlayer")))
    {
        Minigame94_LogAddMinigamePlayer(5, TEXT("%s.AddMinigamePlayer - Skipping %s: Already registered with the minigame."), this, Player);
        return;
    }
    Minigame94_LogAddMinigamePlayer(5, TEXT("%s.AddMinigamePlayer - %s added"), this, Player);

    if (AFortPlayerStateAthena* AthenaPS = Cast<AFortPlayerStateAthena>(Player))
    {
        Minigame94_BindTeamIndexChanged(AthenaPS, this);
    }

    Minigame94_CopyPlayerIdentityToRecord(*Record, Minigame94_MakePlayerIdentity(Player));
    Minigame94_MarkArrayItemDirty(Array, *Record);
    Minigame94_EnsurePlayerSubobjectState(Player);
    Record->SolPlayer = Minigame94_ResolveSolPlayer(Player);

    if (*reinterpret_cast<int32*>(Minigame94_RecordRaw(*Record, 0x80)) == -1)
    {
        Minigame94_TrySetRecordSlotFromController(this, WorldContext, Player, *Record);
    }

    AFortPlayerStateAthena* AthenaPS = Cast<AFortPlayerStateAthena>(Player);
    if (((State == EFortMinigameState::PreGame && !bForced) || State == EFortMinigameState::PostGameReset) || !AthenaPS)
    {
        // falls to the controller back-pointer block below
    }
    else
    {
        Minigame94_SetRecordFlagsAndLocation(*Record, Minigame94_PlayerTeamValue(Player), Pawn); // +0x38,+0x40,+0x44,+0x50,+0x5c in order

        if (!Controller)
        {
            if (!bForced) { Minigame94_NotifyPlayerWithoutController(this, Player); } // 0x1420a7410
            return; // native skips the back-pointer block (controller null)
        }

        if (!bPlaying)
        {
            Record->bIsTeleportingOrRespawningForGameplay = true; // +0x5f
            Minigame94_BindBeginSpectating(AthenaPS, this);
            Minigame94_BeginSpectatingState(Player);
            if (Pawn) { Minigame94_PawnSpectatorAction(Pawn); }
            Minigame94_RefreshSpectatorController(Controller);
        }
        else
        {
            int32 TeamIndex;
            if (Minigame94_IsBucketValid(this, PreferredTeam)) { TeamIndex = PreferredTeam; }
            else { TeamIndex = (uint32)Minigame94_ChooseTeam(this, Player); }
            if (Minigame94_IsBucketValid(this, PreferredTeam))
            {
                Minigame94_AppendPlayerIdentityToBucket(this, PreferredTeam, Player);
            }
            Minigame94_AssignPlayerTeam(this, Player, Minigame94_BucketRoundStartTeam(this, TeamIndex), true);
            if (Record->ClassSlotIndex == 0xff) { Minigame94_ClassSlotDefault(this, Player); }
            if (!bForced) { Minigame94_TouchOptionalSharedState(this); }

            TWeakObjectPtr<AFortPlayerState> WeakPlayer(Player);
            if (!Minigame94_ControllerDefersPlayerSetup(Controller))
            {
                Minigame94_CompletePlayerSetup(this, WeakPlayer);
            }
            else
            {
                Minigame94_ScheduleWeakPlayerSetup(this, WorldContext, WeakPlayer);
            }
            if (Pawn) { Minigame94_BroadcastJoinInProgress(this, Pawn); }
        }
    }

    if (Controller) { Minigame94_SetControllerMinigame(Controller, this); }
}
