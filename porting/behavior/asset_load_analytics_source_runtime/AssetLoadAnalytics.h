#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IAnalyticsProvider.h"

class AFortPlayerController;
class AFortGameStateAthena;
class AFortGameStateZone;
class AFortMission;
class UWorld;

// Integration contracts, named for the original addresses rather than guessing
// private engine API names. Implementations must retain the original dispatch.
namespace AssetLoadAnalyticsNative
{
    // Controller +0x5a0, interface vtable +0x28; returned provider may be null.
    IAnalyticsProvider* const& ProviderSlot14178(const AFortPlayerController& Controller);
    // AFortGameStateAthena primary vtable +0x780, returns float.
    float AthenaMatchTime141769500(const AFortGameStateAthena& GameState);
    // Original 0x14468f490, including streaming-level and package-name logic.
    FString MapName14468f490(const UWorld& World);
    // Mission virtual +0x690 returns an FText; preserve virtual dispatch.
    FText MissionText690(const AFortMission& Mission);
    // Original 0x141e9a800. All four outputs are written by that helper.
    void ZoneValues141e9a800(const AFortGameStateZone& GameState,
        int32& NumberPlayers, float& GameDifficulty, FString& ZoneName, FString& MissionName);

    const TCHAR* AthenaPhase141f90650(uint8 Phase);
    TArray<FAnalyticsEventAttribute> AthenaAttributes141769500(
        TArray<FAnalyticsEventAttribute> Attributes, const UWorld* World);
    void CoreAttributes141763b40(TArray<FAnalyticsEventAttribute>& Attributes, const UWorld* World);
    void AthenaSync14178a860(AFortPlayerController* Controller, const FString& Path);
    void CoreSync14178bba0(AFortPlayerController* Controller, const FString& Path);
    void AthenaAsync14178a4d0(AFortPlayerController* Controller, const FString& Path);
    void CoreAsync14178b860(AFortPlayerController* Controller, const FString& Path);
}
