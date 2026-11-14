// Server-side fall damage for the original 12.60 client (compiled out of the client exe's server path).
#pragma once

#include "CoreMinimal.h"

class UWorld;

namespace FortAthenaFallDamage
{
	// Called once per world tick while the match flow is active. Tracks every Athena pawn's fall speed while it is in
	// ordinary MOVE_Falling and applies fall damage when it lands (curve /Game/Athena/Balance/DataTables/AthenaFallingDamage row Player).
	void Tick(UWorld* World);
}
