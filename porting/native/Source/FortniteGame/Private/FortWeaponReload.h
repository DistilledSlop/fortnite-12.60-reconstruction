#pragma once

#include "CoreMinimal.h"

class AFortPlayerController;
class AFortWeapon;
class UAbilitySystemComponent;

// Server-side reload for ammo weapons.
//
// The retail reload is the native UFortGameplayAbility_Reload: when its montage reaches the load point the Blueprint-callable,
// authority-only AFortWeapon::TryReloadAmmoFromAbility moves rounds from the inventory ammo stack into the replicated magazine
// (AFortWeapon::AmmoCount). Those bodies are stripped from the client executable this port is built from, so the server drives the
// same transfer from the ability activation: it starts a timer for the reload duration, and when the reload is still active and
// the weapon still equipped it fills the magazine from the inventory (or for free with infinite ammo) and ends the ability.
namespace FortWeaponReload
{
	// Watches an ability system component for activations of the reload ability. Safe to call repeatedly for the same component.
	void BindAbilitySystem(UAbilitySystemComponent* ASC);

	// Moves rounds from the inventory ammo stack into the magazine (what the retail AFortWeapon::TryReloadAmmoFromAbility does).
	// Returns whether any round moved.
	bool LoadMagazine(AFortWeapon* Weapon);

	// True when the weapon never runs dry (no ammo use, or the owner has the infinite-ammo rule for its definition).
	bool HasInfiniteAmmo(AFortWeapon* Weapon);

	// One shot's worth of ammo leaves the magazine (and its item entry's loaded-ammo copy).
	void ConsumeShot(AFortPlayerController* Controller, AFortWeapon* Weapon, int32 Cost);
}
