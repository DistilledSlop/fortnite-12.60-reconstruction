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

// Original backing objects registered with flags ECVF_Default.
static int32 GEnableParachuteEverywhere148157cf4 = 0;
static FAutoConsoleVariableRef CVarEnableParachuteEverywhere148157cf4(
    TEXT("Athena.EnableParachuteEverywhere"), GEnableParachuteEverywhere148157cf4,
    TEXT("Whether to enable parachute everywhere or just when skydiving.\n0: Disable, 1: Enable"), ECVF_Default);
static int32 GGliderRedeployRequiresJump148157d08 = 0;
static FAutoConsoleVariableRef CVarGliderRedeployRequiresJump148157d08(
    TEXT("Fort.GliderRedeployRequiresJump"), GGliderRedeployRequiresJump148157d08,
    TEXT("Indicates you must jump first before you can redepoy the glider."), ECVF_Default);
static int32 GGliderRedeployUseWindowOfTime148157d0c = 0;
static FAutoConsoleVariableRef CVarGliderRedeployUseWindowOfTime148157d0c(
    TEXT("Fort.GliderRedeployUseWindowOfTime"), GGliderRedeployUseWindowOfTime148157d0c,
    TEXT("Indicates that glider redeploy will only be open for a window of time."), ECVF_Default);
static float GGliderRedeployWindowLength1478de4b0 = 5.0f;
static FAutoConsoleVariableRef CVarGliderRedeployWindowLength1478de4b0(
    TEXT("Fort.GliderRedeployWindowLength"), GGliderRedeployWindowLength1478de4b0,
    TEXT("Defines the length of time the redeploy glider will is open for."), ECVF_Default);
int32 NativeEnableParachuteEverywhere148157cf4() { return GEnableParachuteEverywhere148157cf4; }
int32 NativeGliderRedeployRequiresJump148157d08() { return GGliderRedeployRequiresJump148157d08; }
int32 NativeGliderRedeployUseWindowOfTime148157d0c() { return GGliderRedeployUseWindowOfTime148157d0c; }
float NativeGliderRedeployWindowLength1478de4b0() { return GGliderRedeployWindowLength1478de4b0; }

// Registrar140264880; original14810e720 is in the PE's virtual-zero .data tail.
static int32 GDisableSkydivingFeedback14810e720 = 0;
static FAutoConsoleVariableRef CVarDisableSkydivingFeedback14810e720(
    TEXT("Athena.DisableSkydivingFeedback"), GDisableSkydivingFeedback14810e720,
    TEXT("Whether the skydive feedback should be enabled.\n1: Disable, 0: Enable"), ECVF_Default);
int32 NativeDisableSkydivingFeedback14810e720() { return GDisableSkydivingFeedback14810e720; }
