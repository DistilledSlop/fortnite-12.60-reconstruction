#include "TargetSelectionVolumeNative.h"
#include "HAL/IConsoleManager.h"

// Original startup registration 0x140245f70, storage 0x1478c1710.
static TAutoConsoleVariable<int32> CVarShouldPerformTargetSelectionVolumeChecks(
    TEXT("Fort.ShouldPerformTargetSelectionVolumeChecks"), 1,
    TEXT("Whether or not to perform volume checks when selecting targets."), ECVF_Default);

bool FortNativeShouldPerformTargetSelectionVolumeChecks()
{
    return CVarShouldPerformTargetSelectionVolumeChecks.GetValueOnGameThread() != 0;
}
