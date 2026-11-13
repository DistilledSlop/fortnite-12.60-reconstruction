#pragma once
#include "CoreMinimal.h"
class AFortGameStateAthena;
class ABuildingActor;

namespace FortBuildingClassCache
{
    // Selects the original level-zero-only filter when bOnlyBaseLevel is true.
    // False means no usable table; does not publish partial results.
    bool Prepare(AFortGameStateAthena* State, bool bOnlyBaseLevel);
    UClass* Resolve(const AFortGameStateAthena* State, uint32 Handle);
}
