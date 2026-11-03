#include "FortAircraftTimingRuntime.h"
#include "HAL/IConsoleManager.h"

// Original writable float 0x1478c2360; registered by 0x1402647a0.
float AircraftClientTimeSmoothing = 0.33f;
static FAutoConsoleVariableRef CVarAircraftClientTimeSmoothing(
    TEXT("Athena.Aircraft.ClientTimeSmoothing"),
    AircraftClientTimeSmoothing,
    TEXT("Amount to smooth aircraft time discrepency to correct for variable latency. 0 disables."),
    ECVF_Default);

// Original writable .data 0x1478c2400, initialized to 1.0f in the supplied PE.
// No console identity is established; retain ordinary mutable process-lifetime storage.
float NativeAircraftTimestampReplicationInterval = 1.0f;
