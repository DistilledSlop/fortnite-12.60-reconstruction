#include "FortParachuteInputNative.h"
#include "HAL/IConsoleManager.h"
// Original registrar14031cc10 binds original148157cfc with flags0.
// Its backing int is zero in the original PE's virtual-zero .data tail.
static int32 GOriginalLimitParachuteDeployRate148157cfc = 0;
static FAutoConsoleVariableRef CVarOriginalLimitParachuteDeployRate148157cfc(
    TEXT("Athena.LimitParachuteDeployRate"),
    GOriginalLimitParachuteDeployRate148157cfc,
    TEXT("Whether to limit the rate at which the parachute can be deployed.\n0: Disable, 1: Enable"),
    ECVF_Default);
int32 NativeLimitParachuteDeployRate148157cfc()
{
    return GOriginalLimitParachuteDeployRate148157cfc;
}
