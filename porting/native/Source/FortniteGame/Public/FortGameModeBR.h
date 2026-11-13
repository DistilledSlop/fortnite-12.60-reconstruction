#pragma once

#include "CoreMinimal.h"
#include "AFortGameModeAthena.h"

#include "FortGameModeBR.generated.h"

// Parent of the original /Game/Athena/Athena_GameMode Blueprint (server-first card C section 6: the Blueprint's
// parent is FortGameModeBR, which has no reflected members of its own). Declared by hand so Athena_GameMode_C
// can load on the dedicated server; behaviour stays in AFortGameModeAthena.
UCLASS(config=Game, transient)
class FORTNITEGAME_API AFortGameModeBR : public AFortGameModeAthena
{
	GENERATED_BODY()
};
