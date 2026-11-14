#include "FortAthenaMovementNative.h"
#include "HAL/IConsoleManager.h"

// Original registrar140274900, int32 reference; original PE initializes it to1.
static int32 GServerDualMoveScopedUpdates1478c2860 = 1;
static FAutoConsoleVariableRef CVarServerDualMoveScopedUpdates1478c2860(
    TEXT("Athena.ServerEnableDualMoveScopedMovementUpdates"), GServerDualMoveScopedUpdates1478c2860,
    TEXT("If enabled, ServerMoveDual will be allowed to wrap both moves in a single ScopedMovementUpdate.\n"),
    ECVF_Default);

bool NativeServerDualMoveScopedUpdates1478c2860()
{
    // The original constructor copies bit0, including for non-Boolean values.
    return (GServerDualMoveScopedUpdates1478c2860 & 1) != 0;
}
