#include "FortAthenaLootWorld.h"

#include "ABuildingContainer.h"
#include "AFortGameModeAthena.h"
#include "AFortGameStateAthena.h"
#include "EAthenaGamePhase.h"
#include "Components/SceneComponent.h"
#include "AFortPickupAthena.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FortAthenaLootData.h"
#include "FortAthenaLootSearch.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/Parse.h"

namespace
{
	const FName DeliveryCrateTag(TEXT("LootDeliveryCrate"));
	const FName GroundCacheTag(TEXT("GroundLootCacheCandidate"));
	constexpr float DeliveryHeight = 1800.0f;
	constexpr float DeliveryDuration = 6.0f;

	struct FScheduledDelivery
	{
		float Delay = 0.0f;
		float LastDescentTick = -1.0f;
		float DescentElapsed = 0.0f;
		FVector LandingLocation = FVector::ZeroVector;
		bool bSearchableCrate = false;
		bool bRestoreCollision = false;
	};

	struct FWorldLootState
	{
		TWeakObjectPtr<UWorld> ActiveWorld;
		TSet<TWeakObjectPtr<ABuildingContainer>> ProcessedContainers;
		TSet<TWeakObjectPtr<ULevel>> ScannedLevels;
		TArray<TWeakObjectPtr<ABuildingContainer>> PendingSpawnRetries;
		TMap<TWeakObjectPtr<ABuildingContainer>, FScheduledDelivery> ScheduledDeliveries;
		TMap<TWeakObjectPtr<ABuildingContainer>, bool> GroundCacheCandidates;
		TMap<FString, TArray<FName>> ConfiguredMarkerTags;
		TMap<FString, TWeakObjectPtr<ABuildingContainer>> SeenConfiguredMarkers;
		int32 ActivatedGroundCacheCount = 0;
		bool bReportedMissingMarkers = false;
		float DeliveryStartTime = -1.0f;
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
		S.ConfiguredMarkerTags.Reset();
		S.SeenConfiguredMarkers.Reset();
		if (GConfig)
		{
			TArray<FString> Settings;
			GConfig->GetArray(TEXT("WorldLoot"), TEXT("MarkerTag"), Settings, GGameIni);
			for (const FString& Setting : Settings)
			{
				FString ActorPath;
				FString Tag;
				if (!Setting.Split(TEXT("|"), &ActorPath, &Tag))
					continue;

				ActorPath.TrimStartAndEndInline();
				Tag.TrimStartAndEndInline();
				if (!ActorPath.IsEmpty() && !Tag.IsEmpty())
					S.ConfiguredMarkerTags.FindOrAdd(ActorPath).AddUnique(FName(*Tag));
			}
		}
		S.ProcessedContainers.Reset();
		S.ScannedLevels.Reset();
		S.PendingSpawnRetries.Reset();
		S.ScheduledDeliveries.Reset();
		S.GroundCacheCandidates.Reset();
		S.ActivatedGroundCacheCount = 0;
		S.bReportedMissingMarkers = false;
		S.DeliveryStartTime = -1.0f;
		S.bLoggedWaitingForData = false;
		UE_LOG(LogTemp, Display, TEXT("LootWorld: reset state for world=%s"), *GetNameSafe(World));
	}

	int32 GetGroundCacheLimit()
	{
		static const int32 Limit = []
		{
			int32 Count = 1;
			FParse::Value(FCommandLine::Get(), TEXT("GroundLootCacheCount="), Count);
			return FMath::Max(0, Count);
		}();
		return Limit;
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
	int32 PopulateCandidates(AFortGameModeAthena* GameMode, const TArray<ABuildingContainer*>& Candidates, const TCHAR* ScopeName, bool bAllowRetry = true)
	{
		FWorldLootState& S = LootWorldState();
		int32 PopulatedMarkers = 0;
		int32 EmptyOutcomes = 0;
		int32 FailedMarkers = 0;
		int32 DuplicateSkipped = 0;
		int32 SpawnedPickups = 0;

		for (ABuildingContainer* Container : Candidates)
		{
			if (!IsValid(Container) || Container->bAlreadySearched || Container->bDestroyed)
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
			const auto Result = FortAthenaLootSearch::RollAndSpawnContainerLoot(GameMode, Container, TierGroup, WorldSpawn, ScatterRadius, &Spawned);
			if (Result != FortAthenaLootSearch::EContainerLootResult::Success)
			{
				++FailedMarkers;
				if (bAllowRetry && Result == FortAthenaLootSearch::EContainerLootResult::SpawnFailed)
					S.PendingSpawnRetries.Add(WeakContainer);
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
	S.ConfiguredMarkerTags.Reset();
	S.SeenConfiguredMarkers.Reset();
	S.ProcessedContainers.Reset();
	S.ScannedLevels.Reset();
	S.PendingSpawnRetries.Reset();
	S.ScheduledDeliveries.Reset();
	S.GroundCacheCandidates.Reset();
	S.ActivatedGroundCacheCount = 0;
	S.bReportedMissingMarkers = false;
	S.DeliveryStartTime = -1.0f;
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
	const auto AddMarker = [&S, &Candidates](ABuildingContainer* Container)
	{
		if (IsValid(Container) && S.ConfiguredMarkerTags.Num() > 0)
		{
			const FString ActorPath = Container->GetPathName();
			if (const TArray<FName>* Tags = S.ConfiguredMarkerTags.Find(ActorPath))
			{
				TWeakObjectPtr<ABuildingContainer>& Seen = S.SeenConfiguredMarkers.FindOrAdd(ActorPath);
				if (Seen.Get() != Container)
				{
					Seen = Container;
					if (IsFloorLootMarkerCandidate(Container))
					{
						for (const FName& Tag : *Tags)
							Container->Tags.AddUnique(Tag);
						UE_LOG(LogTemp, Display, TEXT("LootWorld: configured marker %s tags=%d"), *ActorPath, Tags->Num());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("LootWorld: configured actor %s is not an available world loot marker"), *ActorPath);
					}
				}
			}
		}

		if (!IsFloorLootMarkerCandidate(Container))
			return;

		const TWeakObjectPtr<ABuildingContainer> Marker(Container);
		if (S.ProcessedContainers.Contains(Marker) || S.ScheduledDeliveries.Contains(Marker) ||
			S.GroundCacheCandidates.Contains(Marker))
		{
			return;
		}


		if (Container->ActorHasTag(GroundCacheTag))
		{
			if (S.ActivatedGroundCacheCount >= GetGroundCacheLimit())
				S.ProcessedContainers.Add(Marker);
			else
				S.GroundCacheCandidates.Add(Marker, Container->GetActorEnableCollision());

			Container->SetReplicates(true);
			Container->SetActorEnableCollision(false);
			Container->SetActorHiddenInGame(true);
			Container->ForceNetUpdate();
			return;
		}

		for (const FName& Tag : Container->Tags)
		{
			const FString TagText = Tag.ToString();
			constexpr const TCHAR* Prefix = TEXT("LootDeliverySeconds=");
			if (!TagText.StartsWith(Prefix))
				continue;

			float Delay = 0.0f;
			if (FDefaultValueHelper::ParseFloat(TagText.Mid(FCString::Strlen(Prefix)), Delay) && FMath::IsFinite(Delay) && Delay >= 0.0f)
			{
				FScheduledDelivery Delivery;
				Delivery.Delay = Delay;
				Delivery.bSearchableCrate = Container->ActorHasTag(DeliveryCrateTag);
				if (Delivery.bSearchableCrate)
				{
					const USceneComponent* Root = Container->GetRootComponent();
					if (!Root || Root->Mobility != EComponentMobility::Movable)
					{
						Delivery.bSearchableCrate = false;
						UE_LOG(LogTemp, Warning, TEXT("LootWorld: delivery crate %s needs a movable root, using scheduled pickup delivery"), *Container->GetPathName());
					}
					else
					{
						Delivery.LandingLocation = Container->GetActorLocation();
						Delivery.bRestoreCollision = Container->GetActorEnableCollision();
					}
				}
				S.ScheduledDeliveries.Add(Marker, Delivery);
				if (Delivery.bSearchableCrate)
				{
					Container->SetReplicates(true);
					Container->SetReplicateMovement(true);
					Container->SetActorEnableCollision(false);
					Container->SetActorHiddenInGame(true);
					Container->ForceNetUpdate();
				}
			}
			else
			{
				S.ProcessedContainers.Add(Marker);
				UE_LOG(LogTemp, Warning, TEXT("LootWorld: invalid delivery delay on %s"), *Container->GetName());
			}
			return;
		}

		Candidates.Add(Container);
	};
	if (OptionalLevel)
	{
		if (OptionalLevel->OwningWorld != World)
			return 0;
		for (AActor* Actor : OptionalLevel->Actors)
		{
			AddMarker(Cast<ABuildingContainer>(Actor));
		}
		return PopulateCandidates(GameMode, Candidates, *OptionalLevel->GetOutermost()->GetName());
	}

	for (TActorIterator<ABuildingContainer> It(World); It; ++It)
	{
		AddMarker(*It);
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

	if (S.PendingSpawnRetries.Num() > 0)
	{
		TArray<ABuildingContainer*> RetryCandidates;
		for (const TWeakObjectPtr<ABuildingContainer>& WeakContainer : S.PendingSpawnRetries)
		{
			if (ABuildingContainer* Container = WeakContainer.Get())
			{
				S.ProcessedContainers.Remove(WeakContainer);
				RetryCandidates.Add(Container);
			}
		}
		S.PendingSpawnRetries.Reset();
		PopulateCandidates(GameMode, RetryCandidates, TEXT("spawn retry"), false);
	}

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

	const AFortGameStateAthena* GameState = GameMode->GetGameState<AFortGameStateAthena>();
	if (!GameState)
		return;

	const bool bMatchStarted = GameState->GamePhase == EAthenaGamePhase::Aircraft ||
		GameState->GamePhase == EAthenaGamePhase::SafeZones;
	if (bMatchStarted && !S.bReportedMissingMarkers)
	{
		S.bReportedMissingMarkers = true;
		for (const auto& Entry : S.ConfiguredMarkerTags)
		{
			if (!S.SeenConfiguredMarkers.Contains(Entry.Key))
				UE_LOG(LogTemp, Warning, TEXT("LootWorld: configured marker %s was not present at match start"), *Entry.Key);
		}
	}

	const int32 GroundCacheLimit = GetGroundCacheLimit();
	if (bMatchStarted && S.ActivatedGroundCacheCount < GroundCacheLimit && S.GroundCacheCandidates.Num() > 0)
	{
		TArray<TWeakObjectPtr<ABuildingContainer>> Candidates;
		for (auto It = S.GroundCacheCandidates.CreateIterator(); It; ++It)
		{
			ABuildingContainer* Container = It.Key().Get();
			if (!IsFloorLootMarkerCandidate(Container))
				It.RemoveCurrent();
			else if (Container->GetLevel()->bIsVisible)
				Candidates.Add(It.Key());
		}

		const int32 Count = FMath::Min(GroundCacheLimit - S.ActivatedGroundCacheCount, Candidates.Num());
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Candidates.Swap(Index, FMath::RandRange(Index, Candidates.Num() - 1));
			const TWeakObjectPtr<ABuildingContainer>& Selected = Candidates[Index];
			if (ABuildingContainer* Container = Selected.Get())
			{
				const bool bEnableCollision = S.GroundCacheCandidates.FindChecked(Selected);
				S.GroundCacheCandidates.Remove(Selected);
				S.ProcessedContainers.Add(Selected);
				++S.ActivatedGroundCacheCount;

				Container->bStartAlreadySearched_Athena = false;
				Container->SetActorHiddenInGame(false);
				Container->SetActorEnableCollision(bEnableCollision);
				Container->ForceNetUpdate();
			}
		}

		// Keep pending candidates until the quota is filled by visible levels.
		if (S.ActivatedGroundCacheCount >= GroundCacheLimit)
		{
			for (const auto& Entry : S.GroundCacheCandidates)
				S.ProcessedContainers.Add(Entry.Key);
			S.GroundCacheCandidates.Reset();
		}
	}

	if (GameState->GamePhase != EAthenaGamePhase::SafeZones)
		return;

	const float Now = World->GetTimeSeconds();
	if (S.DeliveryStartTime < 0.0f)
		S.DeliveryStartTime = Now;

	TArray<ABuildingContainer*> DueMarkers;
	for (auto It = S.ScheduledDeliveries.CreateIterator(); It; ++It)
	{
		ABuildingContainer* Marker = It.Key().Get();
		if (!IsFloorLootMarkerCandidate(Marker))
		{
			It.RemoveCurrent();
			continue;
		}

		FScheduledDelivery& Delivery = It.Value();
		if (Now - S.DeliveryStartTime < Delivery.Delay)
			continue;
		if (!Marker->GetLevel()->bIsVisible)
		{
			Delivery.LastDescentTick = -1.0f;
			continue;
		}

		if (!Delivery.bSearchableCrate)
		{
			DueMarkers.Add(Marker);
			It.RemoveCurrent();
			continue;
		}

		if (Delivery.LastDescentTick >= 0.0f)
			Delivery.DescentElapsed += Now - Delivery.LastDescentTick;
		Delivery.LastDescentTick = Now;

		const float Progress = FMath::Clamp(Delivery.DescentElapsed / DeliveryDuration, 0.0f, 1.0f);
		const FVector Target = Delivery.LandingLocation + FVector::UpVector * (DeliveryHeight * (1.0f - Progress));
		Marker->SetActorLocation(Target);
		if (!Marker->GetActorLocation().Equals(Target, 1.0f))
		{
			UE_LOG(LogTemp, Warning, TEXT("LootWorld: delivery crate could not reach its position %s"), *Marker->GetPathName());
			Marker->SetActorHiddenInGame(true);
			Marker->ForceNetUpdate();
			S.ProcessedContainers.Add(It.Key());
			It.RemoveCurrent();
			continue;
		}

		if (Marker->IsHidden())
		{
			Marker->SetActorHiddenInGame(false);
			Marker->ForceNetUpdate();
		}
		if (Progress >= 1.0f)
		{
			Marker->bStartAlreadySearched_Athena = false;
			Marker->SetActorEnableCollision(Delivery.bRestoreCollision);
			Marker->ForceNetUpdate();
			UE_LOG(LogTemp, Display, TEXT("LootWorld: delivery crate landed %s"), *Marker->GetPathName());
			It.RemoveCurrent();
		}
	}
	if (DueMarkers.Num() > 0)
		PopulateCandidates(GameMode, DueMarkers, TEXT("scheduled delivery"));
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
