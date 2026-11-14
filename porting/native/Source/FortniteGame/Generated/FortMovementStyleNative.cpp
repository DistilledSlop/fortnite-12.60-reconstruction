#include "FortMovementStyleNative.h"
#include "HAL/IConsoleManager.h"
// Original .data int 0x1478de49c = 1. Startup registrar 0x14031d2a0
// binds this storage through IConsoleManager +0x38, flags 0.
static int32 GOriginalServerSyncClientSprintAbility1478de49c = 1;
static FAutoConsoleVariableRef CVarOriginalServerSyncClientSprintAbility1478de49c(
    TEXT("Fort.Movement.ServerSyncClientSprintAbility"),
    GOriginalServerSyncClientSprintAbility1478de49c,
    TEXT("Whether server sprint ability syncs on ability tags."),
    ECVF_Default);
bool NativeRequireSprintTag1478de49c()
{
    return GOriginalServerSyncClientSprintAbility1478de49c != 0;
}
