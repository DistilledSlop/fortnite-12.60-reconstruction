#include "FortAssetLoadDiagnosticState.h"
#include "HAL/IConsoleManager.h"
namespace
{
TAutoConsoleVariable<int32> AthenaLimit(
    TEXT("Fort.MaxLoadIssueAnalyticsReportsPerSessionInAthenaShipping"), 20,
    TEXT("The maximum number of analytic events that may be sent from shipping clients in Athena that experience synchronous loads in match.  Can be set to 0 to disable it."), ECVF_Default);
TAutoConsoleVariable<int32> CampaignLimit(
    TEXT("Fort.MaxLoadIssueAnalyticsReportsPerSessionInCampaignShipping"), 20,
    TEXT("The maximum number of analytic events that may be sent from shipping clients in Campaign that experience synchronous loads in match.  Can be set to 0 to disable it."), ECVF_Default);
}
int32 NativeGetLoadIssueAnalyticsLimit(bool bAthena)
{
    return (bAthena ? AthenaLimit : CampaignLimit).AsVariable()->GetInt();
}
