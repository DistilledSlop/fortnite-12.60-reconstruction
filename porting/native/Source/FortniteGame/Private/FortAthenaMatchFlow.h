// G7 match flow for the original 12.60 client: playlist and MapInfo setup, warmup, battle bus, jump, safe zones.
// Hand port of the server code read in porting/server_systems/R1, R2, R4, R5, R6 (addresses in FortAthenaMatchFlow.cpp).
// Plain free functions with file-static state; nothing here adds members to generated classes. Every entry point is a
// no-op unless -FortniteOriginalClient is on the command line.
#pragma once

#include "CoreMinimal.h"

class AController;
class AFortGameModeAthena;
class AFortPlayerController;
class UFortControllerComponent_Aircraft;

namespace FortAthenaMatchFlow
{
    // True once Start() armed the driver (-FortniteOriginalClient set and a game mode/world exist).
    bool IsActive();

    // Call from AFortGameModeAthena::StartPlay, after Super::StartPlay(). Runs the OnPlaylistDataLoaded subset
    // (playlist, AthenaGameData, DefaultMapInfo_C, flight path, storm centres) and arms the per-frame driver that
    // stands in for the unported game mode Tick 0x141975090.
    void Start(AFortGameModeAthena* GameMode);

    // Call after floor/terrain placement and before inventory/health setup can send pawn RPCs and open its actor
    // channel. Places a pending bus-exit pawn below the aircraft without consuming its pending initialization.
    void PositionPlayerForAircraftExit(AFortGameModeAthena* GameMode, AController* Controller);

    // Call at the end of AFortGameModeAthena::RestartPlayer after health initializes the ASC. Applies the storm effect
    // and finishes skydiving initialization for the pawn positioned by PositionPlayerForAircraftExit.
    void OnPlayerRestarted(AFortGameModeAthena* GameMode, AController* Controller);

    // stand-in: binds the re-created jump and ready RPC bodies (see the end of the .cpp). Call once from StartPlay.
    void InstallRpcOverrides();
}

// Global (non-namespaced) entry points so behaviour .inc snippets can reach them with a block-scope declaration
// (snippets cannot add #include lines). Drafts: porting/generated/agents/g7-behavior/.
void FortAthenaMatchFlow_ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* Component, const FRotator& ClientRotation);
void FortAthenaMatchFlow_OnReadyToStartMatch(AFortPlayerController* Controller);
