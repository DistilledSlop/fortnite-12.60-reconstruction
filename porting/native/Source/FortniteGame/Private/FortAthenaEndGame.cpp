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
    bool bReportedUnsupported = false;

    void ResetFor(AFortGameModeAthena* Mode)
    {
        if (ActiveMode.Get() != Mode)
        {
            ActiveMode = Mode;
            bStarted = false;
            bReportedUnsupported = false;
        }
    }

    void BeginSolo(AFortGameModeAthena* Mode, AFortPlayerControllerAthena* Winner,
                   AFortPawn* Finisher, UFortWeaponItemDefinition* Weapon, EDeathCause Cause)
    {
        AFortGameStateAthena* GS = Mode ? Mode->GetGameState<AFortGameStateAthena>() : nullptr;
        AFortPlayerStateAthena* PS = Winner ? Cast<AFortPlayerStateAthena>(Winner->PlayerState) : nullptr;
        UFortPlaylistAthena* Playlist = GS ? GS->GetCurrentPlaylistAthena() : nullptr;
        if (!GS || !PS || !Playlist || bStarted || GS->bForceTeamScorePlacementOnDeath ||
            (GS->GamePhase != EAthenaGamePhase::Aircraft && GS->GamePhase != EAthenaGamePhase::SafeZones))
            return;
        // stand-in: other win-condition modes and team placement require the original mutator and team accounting.
        if (Playlist->MaxTeamSize != 1)
        {
            if (!bReportedUnsupported)
                UE_LOG(LogTemp, Warning, TEXT("EndGame: team and special-mode win checks are not implemented"));
            bReportedUnsupported = true;
            return;
        }
        bStarted = true;
        FortAthenaHealth::FinalizeSurvival(Mode, PS);
        PS->Place = 1;
        PS->OnRep_Place();
        GS->WinningTeam = PS->TeamIndex;
        GS->WinningPlayerState = PS;
        bool bFound = false;
        for (const FFortWinnerPlayerData& Entry : GS->WinningPlayerList)
            bFound |= Entry.PlayerId == PS->GetPlayerId();
        if (!bFound)
        {
            FFortWinnerPlayerData Entry{};
            Entry.PlayerId = PS->GetPlayerId();
            GS->WinningPlayerList.Add(Entry);
        }
        Winner->ClientNotifyWon(Finisher, Weapon, Cause);
        FortAthenaHealth::SendPartialTeamStats(Mode, Winner, 1);
        GS->OnRep_WinningTeam();
        GS->OnRep_WinningPlayerState();
        GS->OnRep_WinningPlayerList();
        const EAthenaGamePhase OldPhase = GS->GamePhase;
        GS->GamePhase = EAthenaGamePhase::EndGame;
        GS->OnRep_GamePhase(OldPhase);
        GS->ForceNetUpdate();
        UE_LOG(LogTemp, Display, TEXT("EndGame: solo winner %s; phase EndGame"), *Winner->GetName());
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
    if (GS && OldPlayersLeft >= 2 && NewPlayersLeft == 1 && GS->PlayersLeft == NewPlayersLeft && Survivor)
        BeginSolo(GameMode, Survivor, FinisherPawn, FinishingWeapon, DeathCause);
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
        BeginSolo(GameMode, Survivor, Cast<AFortPawn>(Survivor->GetPawn()), nullptr, EDeathCause::WonMatch);
}
