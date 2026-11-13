// Server-side player inventory for the original 12.60 client (WP D4, plan G8 first slice).
//
// Everything here is active only when -FortniteOriginalClient is set (IsEnabled()). The functions are free
// functions because generated classes may not get new methods (WORKER_RULES.md). The original server bodies
// (item grant, execute, drop) are compiled out of the client binary, so these are re-creations; each place
// that is not a port of original code carries a "// stand-in:" comment in the .cpp.
#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"

class AFortGameModeAthena;
class AFortPlayerController;
class AFortPlayerControllerAthena;
class AFortWeapon;
class UFortItemDefinition;
class UFortWorldItem;

namespace FortAthenaInventory
{
	// True when the original (vanilla) client is the peer: -FortniteOriginalClient.
	bool IsEnabled();

	// Called once per RestartPlayer, after Super::RestartPlayer (the pawn exists). Idempotent: items are granted
	// once per controller; later calls only re-run the pickaxe equip if the pawn holds no weapon.
	// Grants (in this order): the player's pickaxe, GameMode StartingItems (or the 12.60 defaults from
	// server_systems/C-world-classes-and-abilities.md when the native game mode has none), then equips the pickaxe.
	void SetupPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller);

	// Adds Count of Definition to the controller's WorldInventory (stacks onto an existing entry when the
	// definition is stackable, otherwise creates a new entry with a new FGuid). Returns the world item, or nullptr.
	UFortWorldItem* GiveItem(AFortPlayerController* Controller, UFortItemDefinition* Definition, int32 Count, int32 Level = 1, int32 LoadedAmmo = 0);

	// Removes Count (<= 0 means the whole entry) of the item with this guid. Returns the number removed.
	// Used by the drop/pickup slice; no RPC calls it yet.
	int32 RemoveItem(AFortPlayerController* Controller, const FGuid& ItemGuid, int32 Count);

	// Server side of AFortPlayerController::ServerExecuteInventoryItem: find the entry, equip its definition on the
	// controller's pawn. Unknown guids are ignored (logged).
	void ExecuteInventoryItem(AFortPlayerController* Controller, const FGuid& ItemGuid);

	// stand-in: rebinds the native pointer of the Server RPCs whose generated _Implementation is the folded empty stub
	// (ServerExecuteInventoryItem) to the re-created bodies above, without editing generated code. Call once per world
	// (StartPlay). Remove when the bodies are spliced through behavior/*.inc and the generator.
	void InstallRpcOverrides();
}

// Global-namespace entry points for behaviour snippets (.inc bodies cannot declare a qualified name at block scope).
UFortWorldItem* FortAthenaInventory_GiveItem(AFortPlayerController* Controller, UFortItemDefinition* Definition, int32 Count, int32 Level, int32 LoadedAmmo);
void FortAthenaInventory_ExecuteInventoryItem(AFortPlayerController* Controller, const FGuid& ItemGuid);
