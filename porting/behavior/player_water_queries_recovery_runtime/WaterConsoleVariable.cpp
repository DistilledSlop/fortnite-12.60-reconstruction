// Compile exactly once in FortniteGame, alongside the generated pawn bodies.
#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

int32 GFortPlayerWaterType = 1;
static FAutoConsoleVariableRef FortPlayerWaterTypeVariable(
    TEXT("fx.FortPlayerWaterType"), GFortPlayerWaterType,
    TEXT("If 0, use Cascade. If 1, use Niagara versions of basics. If 2, use procedural Niagara. \n"),
    ECVF_Default);
