#include "FortSwimSprintCVar.h"
#include "HAL/IConsoleManager.h"

static int32 GSwimSprintByDefault148157d00 = 0;
static FAutoConsoleVariableRef CVarSwimSprintByDefault148157d00(
    TEXT("Fort.SwimSprintByDefault"), GSwimSprintByDefault148157d00,
    TEXT("Whether to force auto sprint by default in water when swimming."), ECVF_Default);
int32 NativeSwimSprintByDefaultCVar148157d00()
{
    return GSwimSprintByDefault148157d00;
}
