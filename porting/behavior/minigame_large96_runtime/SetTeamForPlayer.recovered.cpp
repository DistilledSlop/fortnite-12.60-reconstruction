// AFortMinigame::SetTeamForPlayer(AFortPlayerState*, uint8 NewTeam, bool bTemporary, bool bChangeTeamWithoutRespawn) -> bool -- native 0x1420a5820.
// Parameter roles: param_3 = NewTeam (0 => spectate path), param_4 = gate for the bucket-membership rewrite, param_5 = forwarded as the
// 4th delegate arg. Names follow the 23.20 SDK dump (read-only lead) AND the 12.60 native use; 12.60 has fewer params than 23.20.
// SOURCE DRAFT, uncounted, build pending.
#include "Minigame96Bindings.inc"

bool AFortMinigame::SetTeamForPlayer(AFortPlayerState* Player, uint8 NewTeam, bool bTemporary, bool bChangeTeamWithoutRespawn)
{
    if (!Player) { return false; }
    if (GetLocalRole() != ROLE_Authority) { return false; }                   // byte +0xf0 != 3

    const uint8 OldTeam = Minigame94_PlayerTeamValue(Player);                // Player+0x318 vtbl+0x28 (evaluated before the NewTeam==0 test)
    if (NewTeam == 0)
    {
        Minigame96_MoveToSpectate(this, Player);                              // 0x14209b5a0
        Minigame96_RefreshState(this);                                        // 0x142087770
        return true;
    }

    UWorld* World = GetWorld();                                               // vtbl+0x150
    if (!World) { return false; }

    if (UObject* WorldAux = Minigame96_WorldAux(World))                       // 0x141641350 ; null skips validation entirely
    {
        TArray<uint8> Teams;                                                  // pcStack_48 / uStack_40
        if (!Minigame96_GetPlaylistTeams(WorldAux, Teams) || Teams.Num() < 1)   // 0x141996680
        {
            if (Minigame96_LogEnabled(1))
            {
                Minigame96_Logf(2, TEXT("AFortMinigame::SetTeamForPlayer - Failed to get playlist teams. Perhaps the playlist is misconfigured?"));
            }
        }
        else if (!Teams.Contains(NewTeam))
        {
            if (Minigame96_LogEnabled(1))
            {
                Minigame96_Logf(2, TEXT("AFortMinigame::SetTeamForPlayer - Attempting to assign player %s to invalid team %d; falling back to team %d"),
                                *Minigame96_PlayerIdString(Player), (int32)NewTeam, (int32)Teams[0]);   // args inferred
            }
            NewTeam = Teams[0];
        }
        // Teams storage freed here (native FMemory_Free).
    }

    AGameModeBase* GameMode = Minigame96_WorldGameMode(World);                // 0x141641300
    if (GameMode)
    {
        if (Minigame96_LogEnabled(4))
        {
            Minigame96_Logf(5, TEXT("AFortMinigame::SetTeamForPlayer - Attempting to assign player %s to team %d"),
                            *Minigame96_PlayerIdString(Player), (int32)NewTeam);
        }
        AController* Controller = Minigame96_PlayerController(Player);        // 0x142414c60
        const bool bChanged = Minigame96_GameModeSetTeam(GameMode, Controller, NewTeam);   // GM vtbl+0xb28
        if (bChanged)
        {
            if (Minigame96_LogEnabled(4))
            {
                // native: if (Controller) name = Controller+0x590 obj vtbl+0x28(); then string conv + log
                const FString Name = Controller ? Minigame96_ControllerLogName(Controller) : FString();
                Minigame96_Logf(5, TEXT("AFortMinigame::SetTeamForPlayer - Player %s is now on team %d"), *Name, (int32)NewTeam);
            }
            Minigame96_ResetPlayerStat(this, Player, PlayerDeathsStatFilter, 0);   // 0x1420a4190(this, P, this+0x698, 0)
            if (!bTemporary)                                                  // param_4 == 0
            {
                const int32 Count = PlayerBuckets.Num();                      // end pointer fixed before the loop
                for (int32 I = 0; I < Count; ++I)
                {
                    FMinigamePlayerBucket& B = PlayerBuckets[I];
                    if (B.TeamIdAtRoundStart == NewTeam)                      // byte +1
                    {
                        B.PlayerIds.Add(FUniqueNetIdRepl());                  // native appends a null-identity 0x28 element (+8/+0x10/+0x18/+0x20 = 0)
                    }
                    else
                    {
                        Minigame96_RemoveIdentityFromIds(B.PlayerIds, Player->GetUniqueId());   // 0x141e89b00(&PlayerIds, Player+0x248)
                    }
                }
                Minigame96_RefreshBuckets(this);                              // 0x1420a9a30
            }
            PlayerChangedTeamDelegate.Broadcast(Player, NewTeam, OldTeam, bChangeTeamWithoutRespawn);   // 0x1406a9dc0(this+0x308, {P,New,Old,param_5})
            Minigame96_RefreshState(this);                                    // 0x142087770
            return true;                                                      // native returns the GM result byte (nonzero)
        }
    }

    if (Minigame96_LogEnabled(4))
    {
        AController* Controller = Minigame96_PlayerController(Player);
        const FString Name = Controller ? Minigame96_ControllerLogName(Controller) : FString();
        Minigame96_Logf(5, TEXT("AFortMinigame::SetTeamForPlayer - Failed to assign Player %s, remaining on team %d"), *Name, (int32)OldTeam);   // 2nd arg inferred
    }
    return false;
}
