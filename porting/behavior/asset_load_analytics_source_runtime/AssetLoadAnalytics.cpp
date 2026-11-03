#include "AssetLoadAnalytics.h"
#include "AFortGameStateAthena.h"
#include "AFortGameStateZone.h"
#include "AFortPlayerController.h"
#include "AFortMissionManager.h"
#include "AFortMission.h"
#include "UFortMissionInfo.h"
#include "UFortZoneTheme.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

namespace AssetLoadAnalyticsNative
{
const TCHAR* AthenaPhase141f90650(uint8 Phase)
{
    switch (Phase)
    {
    case 0: return TEXT("None");
    case 1: return TEXT("Setup");
    case 2: return TEXT("Warmup");
    case 3: return TEXT("Aircraft");
    case 4: return TEXT("SafeZones");
    case 5: return TEXT("EndGame");
    default: return TEXT("Unknown");
    }
}

TArray<FAnalyticsEventAttribute> AthenaAttributes141769500(
    TArray<FAnalyticsEventAttribute> Attributes, const UWorld* World)
{
    const AFortGameStateAthena* State = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
    if (State)
    {
        // The original obtains both values before appending any attributes.
        const float MatchTime = AthenaMatchTime141769500(*State);
        const uint8 Phase = static_cast<uint8>(State->GamePhase);
        const FString MapName = MapName14468f490(*World);
        Attributes.Emplace(TEXT("AthenaGamePhase"), AthenaPhase141f90650(Phase));
        Attributes.Emplace(TEXT("AthenaMatchTimeSec"), static_cast<double>(MatchTime));
        Attributes.Emplace(TEXT("AthenaMapName"), MapName);
    }
    return Attributes;
}

void ZoneValues141e9a800(const AFortGameStateZone& State, int32& NumberPlayers,
    float& Difficulty, FString& ZoneName, FString& MissionName)
{
    NumberPlayers = 0;
    for (const APlayerState* Player : State.PlayerArray)
        if (Player && !Player->IsSpectator() && !Player->IsOnlyASpectator())
            ++NumberPlayers;
    Difficulty = State.GameDifficulty;
    ZoneName = State.ZoneTheme ? State.ZoneTheme->GetName() : TEXT("<Unavailable>");
    MissionName = TEXT("NullMissionManager");
    if (State.MissionManager)
    {
        // NativeFindPrimaryMission is the separately ported 0x1420bc640.
        if (const AFortMission* Mission = State.MissionManager->NativeFindPrimaryMission())
            MissionName = MissionText690(*Mission).ToString();
    }
}

void CoreAttributes141763b40(TArray<FAnalyticsEventAttribute>& Attributes, const UWorld* World)
{
    const AGameStateBase* State = World ? World->GetGameState() : nullptr;
    const AFortGameStateZone* Zone = Cast<AFortGameStateZone>(State);
    if (Zone && !Cast<AFortGameStateAthena>(State))
    {
        int32 NumberPlayers = -1;
        float Difficulty = -1.0f;
        FString ZoneName, MissionName;
        ZoneValues141e9a800(*Zone, NumberPlayers, Difficulty, ZoneName, MissionName);
        Attributes.Emplace(TEXT("NumberPlayers"), static_cast<double>(NumberPlayers));
        Attributes.Emplace(TEXT("ZoneName"), ZoneName);
        Attributes.Emplace(TEXT("GameDifficulty"), static_cast<double>(Difficulty));
        Attributes.Emplace(TEXT("MissionName"), MissionName);
    }
}

static void Emit(AFortPlayerController* Controller, const FString& Path,
    const TCHAR* Event, bool bAthena)
{
    if (!Controller) return;
    IAnalyticsProvider* const& ProviderSlot = ProviderSlot14178(*Controller);
    IAnalyticsProvider* Provider = ProviderSlot;
    if (!Provider) return;
    TArray<FAnalyticsEventAttribute> Attributes;
    Attributes.Emplace(TEXT("File"), Path);
    UWorld* World = Controller->GetWorld();
    if (bAthena)
        Attributes = AthenaAttributes141769500(MoveTemp(Attributes), World);
    else
        CoreAttributes141763b40(Attributes, World);
    // Core reloads the provider slot after constructing its attributes.
    (bAthena ? Provider : ProviderSlot)->RecordEvent(Event, Attributes);
}

void AthenaSync14178a860(AFortPlayerController* C, const FString& P)
{ Emit(C, P, TEXT("Athena.ClientUnexpectedSynchronousLoad"), true); }
void CoreSync14178bba0(AFortPlayerController* C, const FString& P)
{ Emit(C, P, TEXT("Core.ClientUnexpectedSynchronousLoad"), false); }
void AthenaAsync14178a4d0(AFortPlayerController* C, const FString& P)
{ Emit(C, P, TEXT("Athena.ClientUnexpectedAsynchronousLoad"), true); }
void CoreAsync14178b860(AFortPlayerController* C, const FString& P)
{ Emit(C, P, TEXT("Core.ClientUnexpectedAsynchronousLoad"), false); }
}
