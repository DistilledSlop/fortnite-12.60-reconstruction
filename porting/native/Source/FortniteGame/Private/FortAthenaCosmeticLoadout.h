// Applies the non-character cosmetic slots of the backend loadout (glider, pickaxe, backpack, contrail, emotes, ...)
// to the controller's CosmeticLoadoutPC and the pawn's replicated CosmeticLoadout, like the character already is.
#pragma once

#include "CoreMinimal.h"

class AController;

namespace FortAthenaCosmeticLoadout
{
	// Slots: category (Glider, Pickaxe, Backpack, SkyDiveContrail, Dance, ItemWrap, MusicPack, VictoryPose, LoadingScreen)
	// -> template ids as sent by the backend ("AthenaGlider:glider_id_xxx"). Safe to call repeatedly.
	void Apply(AController* Controller, const TMap<FString, TArray<FString>>& Slots);
}
