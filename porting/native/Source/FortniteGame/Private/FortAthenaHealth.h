// Server-side health, GAS (Ability System Component), attribute sets, damage, and death sequence
// for the original 12.60 vanilla client (WP G8-Health, https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki/Server-First-Scope-Mockup).
//
// Active only when -FortniteOriginalClient is set (IsEnabled()).
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class AActor;
class AController;
class AFortGameModeAthena;
class AFortPawn;
class AFortPlayerControllerAthena;
class AFortPlayerStateAthena;

namespace FortAthenaHealth
{
	// True when the original (vanilla) client is the peer: -FortniteOriginalClient.
	bool IsEnabled();

	// Called once per RestartPlayer (after Super::RestartPlayer, when controller and pawn exist).
	// Idempotent: ensures PlayerState has an ASC, InitAbilityActorInfo(PlayerState, Pawn) is called,
	// attribute sets (UFortRegenHealthSet, UFortDamageSet, etc.) are created on PlayerState/ASC and assigned to Pawn,
	// and GAS_AthenaPlayer abilities/effects are granted to ASC.
	void SetupPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller);
	void RegisterAlivePlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller);

	// Applies damage to a pawn's HealthSet (Shield first, then Health; or bIgnoreShield for storm/fall damage).
	// Sends NetMulticast_Athena_BatchedDamageCues so the client displays hit markers and damage numbers.
	// Calls HandlePlayerEliminated if health drops to 0.
	void ApplyDamage(AFortPawn* VictimPawn, float DamageAmount, const FGameplayTagContainer& DamageTags,
	                 AController* InstigatedBy = nullptr, AActor* DamageCauser = nullptr,
	                 bool bIgnoreShield = false, bool bEmitHitCue = true);

	bool TryStartRevive(AController* Reviver, AFortPawn* Target);
	bool TryCarryDownedPlayer(AController* Carrier, AFortPawn* Target);

	// Runs the server death sequence (D6 §2): death report, DeathInfo replication, ClientOnPawnDied,
	// alive player removal (R3), placement calculation, client match/team stats, and win check.
	void HandlePlayerEliminated(AFortPawn* VictimPawn, const FGameplayTagContainer& DamageTags,
	                            AController* InstigatedBy = nullptr, AActor* DamageCauser = nullptr, float LethalDamage = 0.0f);

	// Stand-in RPC overrides (e.g. ServerSuicide, ServerResetAbilitySystemComponent) rebound via UFunction::SetNativeFunc.
	void InstallRpcOverrides();

	// Freeze the starting roster and participation clock at the reconstructed aircraft boundary.
	void BeginMatch(AFortGameModeAthena* GameMode);
	AFortPlayerControllerAthena* GetSoleSurvivor(AFortGameModeAthena* GameMode);
	// Team modes: fills OutTeam with every alive player and returns true when they all share one team (and at least one is alive).
	bool GetSingleAliveTeam(AFortGameModeAthena* GameMode, TArray<AFortPlayerControllerAthena*>& OutTeam);
	// Number of distinct teams with at least one alive player.
	int32 GetAliveTeamCount(AFortGameModeAthena* GameMode);
	void FinalizeSurvival(AFortGameModeAthena* GameMode, AFortPlayerStateAthena* PlayerState);
	void SendPartialTeamStats(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller, int32 Place, bool bPadTotal = false);

	// Returns the current count of alive player controllers.
	int32 GetAlivePlayerCount();
}

// Global-namespace entry points for behaviour snippets (.inc bodies).
void FortAthenaHealth_ApplyDamage(AFortPawn* VictimPawn, float DamageAmount, const FGameplayTagContainer& DamageTags,
                                 AController* InstigatedBy = nullptr, AActor* DamageCauser = nullptr, bool bIgnoreShield = false);
void FortAthenaHealth_HandlePlayerEliminated(AFortPawn* VictimPawn, const FGameplayTagContainer& DamageTags,
                                            AController* InstigatedBy = nullptr, AActor* DamageCauser = nullptr);
