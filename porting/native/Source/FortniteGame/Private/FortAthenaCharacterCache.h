#pragma once
#include "CoreMinimal.h"

class AController;

namespace FortAthenaCharacterCache
{
    // Load and SHA-1 verify Config/AthenaCharacterRegistry.json (fail-closed on mismatch).
    bool Prepare();

    // Returns true only when the character is ready to spawn.
    // Apply verified registered character to Controller + PlayerState (and Pawn if already spawned).
    // Preserves non-registered mode untouched. Fail-closed on malformed/missing loadout or asset.
    bool ApplyForController(AController* Controller, bool bAfterPawnSpawn);
}
