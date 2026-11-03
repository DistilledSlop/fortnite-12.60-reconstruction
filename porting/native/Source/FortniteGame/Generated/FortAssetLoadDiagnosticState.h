#pragma once
#include "CoreMinimal.h"
// The actual FFortAssets diagnostic substate, originally +1d0..+1f4.
// Full owner construction and delegate installation remain separately open.
struct FFortAssetLoadDiagnosticState
{
    double LastLoadTime = 0.0;
    FString LastLoadPath;
    int32 AthenaLoadIssueCount = 0;
    int32 CampaignLoadIssueCount = 0;
};
FORTNITEGAME_API int32 NativeGetLoadIssueAnalyticsLimit(bool bAthena);
