#pragma once
#include "CoreMinimal.h"
class AFortGameStateAthena;
class ABuildingActor;

namespace FortBuildingClassCache
{
    // Default all-level scan only. False means no usable table; does not publish partial results.
    bool Prepare(AFortGameStateAthena* State);
    UClass* Resolve(const AFortGameStateAthena* State, uint32 Handle);
}
