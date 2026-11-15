#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

class AActor;
class ABuildingActor;
class ABuildingSMActor;
class AController;

namespace FortHarvest
{
	/** Returns true when the server runs with -FortniteOriginalClient. */
	bool IsEnabled();

	/** Ensures 12.60 building attribute tables (AttributesBuildingProps, AthenaAttributesBuildingSection) are preloaded for lazy prop health lookup. */
	void EnsureBuildingAttributeDataLoaded();

	/** Returns the building's current health (BuildingAttributeSet->Health when instantiated, otherwise GetBuildingMaxHealth). */
	float GetBuildingHealth(const ABuildingActor* Building);

	/** Returns the building's max health (BuildingAttributeSet->MaxHealth, cached MaxHealthInitializationValue, or FFortAttributeSetInitter curve lookup). */
	float GetBuildingMaxHealth(ABuildingActor* Building);

	/** Initializes a building's max health and current health percentage (level props = 100%, player-placed = curve percentage). */
	bool InitializeBuildingHealth(ABuildingActor* Building);

	/** Materializes AbilitySystemComponent / BuildingAttributeSet if lazy and forces Health to NewHealth [0, MaxHealth]. */
	void ForceBuildingHealth(ABuildingActor* Building, float NewHealth);

	/**
	 * Replicates the damage presentation of a building: sets ABuildingSMActor::ProxyGameplayCueDamagePhysical, whose client
	 * OnRep (0x142757c10) turns the magnitude into a damage cue (cracks, hit reaction) and, when bFatal, into the death cue
	 * (client death presentation, removal). The sign of the magnitude alternates per hit because the client reads abs() and
	 * the property only replicates when its value changes.
	 */
	void ReplicateBuildingDamageCue(
		ABuildingSMActor* Building,
		float Damage,
		AController* Instigator,
		AActor* Causer,
		const FHitResult& Hit,
		bool bFatal);

	/** Applies harvesting/combat damage to a building/prop: health reduction, Blueprint/delegate notifications, resource grant + overflow drop, client RPCs, weak spots, and destruction. */
	bool ApplyBuildingDamage(
		ABuildingSMActor* Building,
		float Damage,
		AController* Instigator,
		AActor* Causer,
		const FHitResult& Hit,
		bool bWeakSpot);
}
