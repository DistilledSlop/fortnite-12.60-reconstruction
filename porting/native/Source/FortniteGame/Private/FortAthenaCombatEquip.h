#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"

class AFortPawn;
class AFortPlayerState;
class AFortWeapon;
class UFortGameplayAbility;
class UFortAbilitySet;

namespace FortAthenaCombatEquip
{
	// Slots 0..3 are primary, secondary, reload and on-hit. Subsequent
	// slots represent EquippedAbilities, preserving original order.
	struct FGrant
	{
		TSubclassOf<UFortGameplayAbility> Ability;
		int32 Slot;
	};

	// Retryable and idempotent for the same weapon, avatar and grant list.
	// Call only after resolving all definition fields and their input policy.
	bool EnsureGrants(AFortPlayerState* Owner, AFortPawn* Avatar, AFortWeapon* Weapon,
		const TArray<FGrant>& Grants, UFortAbilitySet* EquippedSet, bool bAuditFromTick);
	// Athena switch rule. Returns false if an ability-set lifetime is unresolved.
	bool OnUnequip(AFortWeapon* Weapon);
	void RemoveWeapon(AFortWeapon* Weapon);
	// Run on the game thread during inventory tick, including after possession changes.
	void Prune();
}
