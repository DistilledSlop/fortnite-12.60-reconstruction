#include "FortAthenaEndGame.h"
#include "FortAthenaHealth.h"
#include "AFortGameModeAthena.h"
#include "AFortGameStateAthena.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerStateAthena.h"
#include "AFortPawn.h"
#include "UFortPlaylistAthena.h"
#include "FFortWinnerPlayerData.h"
#include "EAthenaGamePhase.h"
#include "EDeathCause.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    TWeakObjectPtr<AFortGameModeAthena> ActiveMode;
    bool bStarted = false;
    
    void ResetFor(AFortGameModeAthena* Mode)
    {
        if (ActiveMode.Get() != Mode)
        {
            ActiveMode = Mode;
            bStarted = false;
        }
    }

    void BeginWin(AFortGameModeAthena* Mode, const TArray<AFortPlayerControllerAthena*>& Winners,
                  AFortPawn* Finisher, UFortWeaponItemDefinition* Weapon, EDeathCause Cause)
    {
        AFortPlayerControllerAthena* Winner = Winners.Num() > 0 ? Winners[0] : nullptr;
        AFortGameStateAthena* GS = Mode ? Mode->GetGameState<AFortGameStateAthena>() : nullptr;
        AFortPlayerStateAthena* PS = Winner ? Cast<AFortPlayerStateAthena>(Winner->PlayerState) : nullptr;
        UFortPlaylistAthena* Playlist = GS ? GS->GetCurrentPlaylistAthena() : nullptr;
        if (!GS || !PS || !Playlist || bStarted || GS->bForceTeamScorePlacementOnDeath ||
            (GS->GamePhase != EAthenaGamePhase::Aircraft && GS->GamePhase != EAthenaGamePhase::SafeZones))
            return;
        // stand-in: other special win-condition modes need the original mutator accounting. Solo and last-team-standing
        // (duo/squad) are handled; the original team placement and results reporting are not reconstructed.
        bStarted = true;
        GS->WinningTeam = PS->TeamIndex;
        GS->WinningPlayerState = PS;
        for (AFortPlayerControllerAthena* WinnerPC : Winners)
        {
            AFortPlayerStateAthena* WinnerState = WinnerPC ? Cast<AFortPlayerStateAthena>(WinnerPC->PlayerState) : nullptr;
            if (!WinnerState)
                continue;
            FortAthenaHealth::FinalizeSurvival(Mode, WinnerState);
            WinnerState->Place = 1;
            WinnerState->OnRep_Place();
            bool bFound = false;
            for (const FFortWinnerPlayerData& Entry : GS->WinningPlayerList)
                bFound |= Entry.PlayerId == WinnerState->GetPlayerId();
            if (!bFound)
            {
                FFortWinnerPlayerData Entry{};
                Entry.PlayerId = WinnerState->GetPlayerId();
                GS->WinningPlayerList.Add(Entry);
            }
            WinnerPC->ClientNotifyWon(Finisher, Weapon, Cause);
            FortAthenaHealth::SendPartialTeamStats(Mode, WinnerPC, 1);
        }
        GS->OnRep_WinningTeam();
        GS->OnRep_WinningPlayerState();
        GS->OnRep_WinningPlayerList();
        const EAthenaGamePhase OldPhase = GS->GamePhase;
        GS->GamePhase = EAthenaGamePhase::EndGame;
        GS->OnRep_GamePhase(OldPhase);
        GS->ForceNetUpdate();
        UE_LOG(LogTemp, Display, TEXT("EndGame: winner team %d (%d players, first %s); phase EndGame"), PS->TeamIndex, Winners.Num(), *Winner->GetName());
        // stand-in: the original end-match helper, results reporting and delayed return are not reconstructed here.
        UE_LOG(LogTemp, Warning, TEXT("EndGame: end-match helper, results reporting, post-game return and shutdown are unresolved"));
    }
}

bool FortAthenaEndGame::IsEnabled()
{
    return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

void FortAthenaEndGame::OnPlayerEliminated(AFortGameModeAthena* GameMode, int32 OldPlayersLeft, int32 NewPlayersLeft,
    AFortPlayerControllerAthena* Survivor, AFortPawn* FinisherPawn,
    UFortWeaponItemDefinition* FinishingWeapon, EDeathCause DeathCause)
{
    if (!IsEnabled() || !GameMode) return;
    ResetFor(GameMode);
    AFortGameStateAthena* GS = GameMode->GetGameState<AFortGameStateAthena>();
    TArray<AFortPlayerControllerAthena*> Team;
    if (GS && NewPlayersLeft >= 1 && OldPlayersLeft > NewPlayersLeft && GS->PlayersLeft == NewPlayersLeft &&
        FortAthenaHealth::GetSingleAliveTeam(GameMode, Team) &&
        // solo playlists end on the last player regardless of how team indices were assigned
        (!GS->GetCurrentPlaylistAthena() || GS->GetCurrentPlaylistAthena()->MaxTeamSize != 1 || Team.Num() == 1))
        BeginWin(GameMode, Team, FinisherPawn, FinishingWeapon, DeathCause);
}

void FortAthenaEndGame::Tick(AFortGameModeAthena* GameMode, float DeltaSeconds)
{
    (void)DeltaSeconds;
    if (!IsEnabled() || !GameMode || !GameMode->GetWorld()) return;
    ResetFor(GameMode);
    if (bStarted || !FParse::Param(FCommandLine::Get(), TEXT("FortniteQuickEndGame"))) return;
    AFortGameStateAthena* GS = GameMode->GetGameState<AFortGameStateAthena>();
    if (!GS || GS->PlayersLeft != 1 ||
        (GS->GamePhase != EAthenaGamePhase::Aircraft && GS->GamePhase != EAthenaGamePhase::SafeZones)) return;
    // stand-in: the testing switch permits one player without a prior elimination. Reject ambiguous rosters.
    AFortPlayerControllerAthena* Survivor = FortAthenaHealth::GetSoleSurvivor(GameMode);
    if (Survivor)
        BeginWin(GameMode, TArray<AFortPlayerControllerAthena*>{ Survivor }, Cast<AFortPawn>(Survivor->GetPawn()), nullptr, EDeathCause::WonMatch);
}
