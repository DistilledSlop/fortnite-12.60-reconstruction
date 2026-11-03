// Rough typed proposal for native 0x142081fa0; NOT a canonical .inc.
// The Minigame94_* operations are unresolved, ordered dependencies, not stubs.
// See README.md for contracts and byte offsets. Objects and records remain owned
// by their original game systems; local handles below do not transfer ownership.
void AddMinigamePlayer_Recovered(AFortMinigame* Game, AFortPlayerState* Player, bool bForced)
{
    if (!Minigame94_IsAuthority(Game)) return;
    if (!Player) { Minigame94_LogInvalidPlayer(Game); return; }
    const bool bManagerOverride = Minigame94_ManagerAllowsImmediateJoin(Game);
    const int32 Mode = Minigame94_Mode(Game); // +0xb10
    const bool bPlaying = (Mode == 2 || Mode == 3 || bForced || bManagerOverride);
    const uint8 PreferredTeam = Mode == 3 ? Minigame94_PreferredTeam(Game) : 0xff;
    auto* WorldContext = Minigame94_GetWorldContext(Game); // virtual +0x150
    if (!WorldContext) return;
    const uint8 State = Minigame94_State(Game); // +0x870
    auto& Array = (State == 0 || bPlaying || State == 10)
        ? Minigame94_PlayerArray(Game) : Minigame94_SpectatorArray(Game);
    auto* Entry = Minigame94_FindOrCreatePlayerRecord(Array, Player);
    if (!Entry) return;
    auto* Pawn = Minigame94_PlayerPawn(Player); // native 0x1418563c0
    auto* Controller = Minigame94_AthenaController(Player);
    if (Minigame94_RejectLateJoinEnabled() && Controller &&
        Minigame94_RejectLateJoin(Game) && State > 5 && State != 10)
    {
        Minigame94_SendLateJoinMessage(Controller,
            TEXT("The game you were tring to join had already ended."),
            TEXT("FortMinigame"), TEXT("JoinInProgressTooLate"));
        return;
    }
    if (Minigame94_IsRegistered(Entry, WorldContext, TEXT("AFortMinigame::AddMinigamePlayer")))
    {
        Minigame94_LogAlreadyRegistered(Game, Player);
        return;
    }
    Minigame94_LogPlayerAdded(Game, Player);
    if (Minigame94_IsAthenaPlayerState(Player))
        Minigame94_BindTeamIndexChanged(Player, Game);
    Minigame94_CopyPlayerIdentityToRecord(Entry, Player);
    Minigame94_MarkArrayItemDirty(Array, Entry);
    Minigame94_EnsurePlayerSubobjectState(Player);
    Minigame94_SetRecordPlayerReference(Entry, Player);
    if (Minigame94_RecordSlotIndex(Entry) == -1)
        Minigame94_TrySetRecordSlotFromController(Game, WorldContext, Player, Entry);

    if ((State == 0 && !bForced) || State == 10)
    {
        Minigame94_SetControllerMinigame(Controller, Game);
        return;
    }
    if (!Minigame94_IsAthenaPlayerState(Player))
    {
        Minigame94_SetControllerMinigame(Controller, Game);
        return;
    }
    Minigame94_SetRecordActive(Entry, true);
    Minigame94_SetRecordPlayerValue(Entry, Player); // virtual from Player+0x318
    if (Pawn) Minigame94_SavePawnTransform(Entry, Pawn);
    Minigame94_SetRecordInitialized(Entry, true);
    if (!Controller)
    {
        if (!bForced) Minigame94_NotifyPlayerWithoutController(Game, Player);
        return;
    }
    if (!bPlaying)
    {
        Minigame94_SetRecordSpectating(Entry, true);
        Minigame94_BindBeginSpectating(Player, Game);
        Minigame94_BeginSpectating(Player);
        if (Pawn) Minigame94_ApplySpectatorPawnAction(Pawn);
        Minigame94_RefreshSpectatorController(Controller);
        Minigame94_SetControllerMinigame(Controller, Game);
        return;
    }
    const int32 TeamIndex = Minigame94_IsTeamIndexValid(Game, PreferredTeam)
        ? PreferredTeam : Minigame94_ChooseTeam(Game, Player);
    if (Minigame94_IsTeamIndexValid(Game, PreferredTeam))
        Minigame94_AppendPlayerIdentityToTeam(Game, PreferredTeam, Player);
    Minigame94_AssignPlayerTeam(Game, Player, TeamIndex, true);
    if (Minigame94_RecordTeam(Entry) == 0xff)
        Minigame94_UpdatePlayerTeam(Game, Player);
    if (!bForced) Minigame94_TouchOptionalSharedState(Game);
    if (!Minigame94_ControllerDefersPlayerSetup(Controller))
        Minigame94_CompletePlayerSetup(Game, Player);
    else
        Minigame94_ScheduleWeakPlayerSetup(Game, WorldContext, Player);
    if (Pawn) Minigame94_AddPawnToGameList(Game, Pawn);
    Minigame94_SetControllerMinigame(Controller, Game);
}
