#include "FortBuildingFoundationRuntime.h"
#include "HAL/IConsoleManager.h"

// Original PE .data virtual-tail storage is zero initialized.
int32 GNativeFoundationUseStreamingDataBounds = 0;

// Original initializer 0x140295b50; RAII unregistration corresponds to its atexit destructor.
static FAutoConsoleVariableRef CVarNativeFoundationUseStreamingDataBounds(
    TEXT("Fort.BuildingFoundationUseStreamingDataBounds"),
    GNativeFoundationUseStreamingDataBounds,
    TEXT("Whether to use streaming data bounds when calculating the bounds of the foundation. When disabled, it uses the bounds of the boundation actor itself."),
    ECVF_Default);
