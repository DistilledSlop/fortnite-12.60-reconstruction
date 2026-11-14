#include "FortAthenaLootWorld.h"

#include "ABuildingContainer.h"
#include "AFortGameModeAthena.h"
#include "AFortPickupAthena.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FortAthenaLootData.h"
#include "FortAthenaLootSearch.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
	struct FWorldLootState
	{
		TWeakObjectPtr<UWorld> ActiveWorld;
		TSet<TWeakObjectPtr<ABuildingContainer>> ProcessedContainers;
		TSet<TWeakObjectPtr<ULevel>> ScannedLevels;
		bool bLoggedWaitingForData = false;
	};

	FWorldLootState& LootWorldState()
	{
		static FWorldLootState Instance;
		return Instance;
	}

	void SyncWorld(UWorld* World)
	{
		FWorldLootState& S = LootWorldState();
		if (S.ActiveWorld.Get() == World)
			return;

		S.ActiveWorld = World;
		S.ProcessedContainers.Reset();
		S.ScannedLevels.Reset();
		S.bLoggedWaitingForData = false;
		UE_LOG(LogTemp, Display, TEXT("LootWorld: reset state for world=%s"), *GetNameSafe(World));
	}

	bool IsFloorLootMarkerCandidate(const ABuildingContainer* Container)
	{
		if (!IsValid(Container) || Container->IsPendingKill() || Container->bDestroyed)
			return false;

		// Cooked data fact: Tiered_Athena_FloorLoot_01_C and Tiered_Athena_FloorLoot_Warmup_C set
		// bStartAlreadySearched_Athena=True, whereas chests and ammo boxes leave it False.
		if (!Container->bStartAlreadySearched_Athena)
			return false;

		if (Container->bAlreadySearched)
			return false;

		return FortAthenaLootSearch::ResolveContainerLootTierGroup(Container) != NAME_None;
	}

	// stand-in: populates markers immediately once loot data is ready and destroys bDestroyContainerOnSearch markers
	// rather than reproducing the unrecovered FortWorldManager / match-phase floor-loot batch schedule.
	int32 PopulateCandidates(AFortGameModeAthena* GameMode, const TArray<ABuildingContainer*>& Candidates, const TCHAR* ScopeName)
	{
		FWorldLootState& S = LootWorldState();
		int32 PopulatedMarkers = 0;
		int32 EmptyOutcomes = 0;
		int32 FailedMarkers = 0;
		int32 DuplicateSkipped = 0;
		int32 SpawnedPickups = 0;

		for (ABuildingContainer* Container : Candidates)
		{
			if (!IsValid(Container) || Container->IsPendingKill())
				continue;

			const TWeakObjectPtr<ABuildingContainer> WeakContainer(Container);
			if (S.ProcessedContainers.Contains(WeakContainer))
			{
				++DuplicateSkipped;
				continue;
			}

			S.ProcessedContainers.Add(WeakContainer);

			const FName TierGroup = FortAthenaLootSearch::ResolveContainerLootTierGroup(Container);
			if (TierGroup == NAME_None)
				continue;

			const FVector WorldSpawn = Container->GetActorTransform().TransformPosition(Container->LootSpawnLocation_Athena);
			const float ScatterRadius = Container->SearchBounceRadiusOverride > 0.0f
				? FMath::Min(Container->SearchBounceRadiusOverride * 0.25f, 50.0f)
				: 35.0f; // stand-in: compact multi-drop offset for weapon+ammo pairs

			TArray<AFortPickupAthena*> Spawned;
			if (!FortAthenaLootSearch::RollAndSpawnContainerLoot(GameMode, Container, TierGroup, WorldSpawn, ScatterRadius, &Spawned))
			{
				++FailedMarkers;
				continue;
			}

			++PopulatedMarkers;
			SpawnedPickups += Spawned.Num();
			if (Spawned.Num() == 0)
				++EmptyOutcomes;

			Container->bAlreadySearched = 1;
			Container->ForceNetUpdate();
			if (Container->bDestroyContainerOnSearch && !Container->bAlwaysShowContainer)
				Container->Destroy();
		}

		if (Candidates.Num() > 0 || DuplicateSkipped > 0)
		{
			UE_LOG(LogTemp, Display, TEXT("LootWorld: scope=%s markers=%d populated=%d empty=%d failed=%d dup_skipped=%d pickups=%d"),
				ScopeName, Candidates.Num(), PopulatedMarkers, EmptyOutcomes, FailedMarkers, DuplicateSkipped, SpawnedPickups);
		}
		return PopulatedMarkers;
	}
}

bool FortAthenaLootWorld::IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

void FortAthenaLootWorld::Reset()
{
	FWorldLootState& S = LootWorldState();
	S.ActiveWorld.Reset();
	S.ProcessedContainers.Reset();
	S.ScannedLevels.Reset();
	S.bLoggedWaitingForData = false;
}

int32 FortAthenaLootWorld::PopulateLoadedMarkers(AFortGameModeAthena* GameMode, ULevel* OptionalLevel)
{
	if (!IsEnabled() || !GameMode || !GameMode->HasAuthority())
		return 0;

	UWorld* World = GameMode->GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
		return 0;

	SyncWorld(World);
	FWorldLootState& S = LootWorldState();

	if (!FortAthenaLootData::IsReady(GameMode))
	{
		if (!S.bLoggedWaitingForData)
		{
			S.bLoggedWaitingForData = true;
			UE_LOG(LogTemp, Display, TEXT("LootWorld: deferred population for world=%s (FortAthenaLootData not ready)"), *World->GetName());
		}
		return 0;
	}

	S.bLoggedWaitingForData = false;

	TArray<ABuildingContainer*> Candidates;
	if (OptionalLevel)
	{
		if (OptionalLevel->OwningWorld != World)
			return 0;
		for (AActor* Actor : OptionalLevel->Actors)
		{
			if (ABuildingContainer* Container = Cast<ABuildingContainer>(Actor))
			{
				if (IsFloorLootMarkerCandidate(Container))
					Candidates.Add(Container);
			}
		}
		return PopulateCandidates(GameMode, Candidates, *OptionalLevel->GetOutermost()->GetName());
	}

	for (TActorIterator<ABuildingContainer> It(World); It; ++It)
	{
		if (IsFloorLootMarkerCandidate(*It))
			Candidates.Add(*It);
	}
	return PopulateCandidates(GameMode, Candidates, *World->GetName());
}

void FortAthenaLootWorld::Tick(AFortGameModeAthena* GameMode)
{
	if (!IsEnabled() || !GameMode || !GameMode->HasAuthority())
		return;

	UWorld* World = GameMode->GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
		return;

	SyncWorld(World);
	FWorldLootState& S = LootWorldState();

	if (!FortAthenaLootData::IsReady(GameMode))
	{
		if (!S.bLoggedWaitingForData)
		{
			S.bLoggedWaitingForData = true;
			UE_LOG(LogTemp, Display, TEXT("LootWorld: waiting for FortAthenaLootData readiness"));
		}
		return;
	}

	S.bLoggedWaitingForData = false;

	// Visit each visible level once (catches persistent + already-loaded levels + streamed-in POIs).
	const TArray<ULevel*>& Levels = World->GetLevels();
	for (ULevel* Level : Levels)
	{
		if (!Level || !Level->bIsVisible)
			continue;
		const TWeakObjectPtr<ULevel> WeakLevel(Level);
		if (S.ScannedLevels.Contains(WeakLevel))
			continue;

		S.ScannedLevels.Add(WeakLevel);
		PopulateLoadedMarkers(GameMode, Level);
	}
}

void FortAthenaLootWorld::OnLevelAdded(AFortGameModeAthena* GameMode, ULevel* Level)
{
	if (!IsEnabled() || !GameMode || !GameMode->HasAuthority() || !Level)
		return;

	UWorld* World = GameMode->GetWorld();
	if (!World || World->GetNetMode() == NM_Client || Level->OwningWorld != World)
		return;

	SyncWorld(World);
	FWorldLootState& S = LootWorldState();

	if (!FortAthenaLootData::IsReady(GameMode) || !Level->bIsVisible)
	{
		UE_LOG(LogTemp, Display, TEXT("LootWorld: level added=%s queued for Tick (lootReady=%d visible=%d)"),
			*Level->GetOutermost()->GetName(), FortAthenaLootData::IsReady(GameMode) ? 1 : 0, Level->bIsVisible ? 1 : 0);
		return;
	}

	S.ScannedLevels.Add(TWeakObjectPtr<ULevel>(Level));
	PopulateLoadedMarkers(GameMode, Level);
}
