#include "FortAthenaCVarQueriesRuntime.h"
#include "HAL/IConsoleManager.h"

// Original .data 0x1478df4c8; registration at 0x14035f300.
int32 FortSkyTubesEnabled = 1;
static FAutoConsoleVariableRef CVarFortSkyTubesEnabled(
    TEXT("Fort.SkyTubesEnabled"), FortSkyTubesEnabled,
    TEXT("Whether tubes are enabled at all."), ECVF_Default);
