#include "AssetLoadAnalytics.h"
#include "AFortMission.h"
#include "AFortGameStateAthena.h"

namespace AssetLoadAnalyticsNative
{
FText MissionText690(const AFortMission& Mission)
{
    return Mission.NativeMissionText690();
}

float AthenaMatchTime141769500(const AFortGameStateAthena& GameState)
{
    return GameState.NativeMatchTime780();
}
}
