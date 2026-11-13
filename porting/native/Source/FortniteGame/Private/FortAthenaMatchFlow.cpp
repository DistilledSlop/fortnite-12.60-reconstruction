// G7 match flow, hand port of the original server code for the 12.60 client (gate G7 in porting/SERVER_FIRST_PLAN.md).
//
// Sources (line-by-line reads of the client image, addresses in the shipping exe):
//   R1 = porting/server_systems/R1-warmup-and-aircraft-code.md     (tick driver, warmup, aircraft phase, bus delegates)
//   R2 = porting/server_systems/R2-safe-zone-code.md               (IsSafeZoneAllowed, StartSafeZonesPhase, storm update)
//   R4 = porting/server_systems/R4-readiness-build-interact-code.md (OnReadyToStartMatch)
//   R5 = porting/server_systems/R5-playlist-setup-phase-handler-receipts.md (flight path, storm centres, phase handler)
//   R6 = porting/server_systems/R6-playlist-trigger-storm-table-storm-damage.md (MapInfo fill, storm damage per pawn)
//   D5 = porting/server_systems/D5-stubbed-rpcs-and-recreation.md section 3 (bus jump re-creation design)
//
// Conventions: "port" comments name the original function; `// stand-in:` marks behaviour that is not a port of
// original code (unread formula, unported callee, or a test convenience). Unknowns are logged once with UE_LOG.
// Everything is inactive unless -FortniteOriginalClient is set.

#include "FortAthenaMatchFlow.h"
#include "EngineUtils.h"
#include "FortAthenaInventoryFlow.h"
#include "FortAthenaHealth.h"
#include "FortAthenaEndGame.h"
#include "Engine/LevelStreaming.h"
#include "LandscapeProxy.h"

#include "AFortGameModeAthena.h"
#include "AFortGameStateAthena.h"
#include "AFortAthenaMapInfo.h"
#include "AFortAthenaAircraft.h"
#include "AFortSafeZoneIndicator.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerController.h"
#include "AFortPlayerStateZone.h"
#include "AFortPlayerPawn.h"
#include "AFortPawn.h"
#include "UFortPlaylistAthena.h"
#include "FortAthenaLootData.h"
#include "UFortControllerComponent_Aircraft.h"
#include "FAircraftFlightInfo.h"
#include "FFortSafeZoneDefinition.h"
#include "EAthenaGamePhase.h"
#include "EAthenaGamePhaseStep.h"
#include "ESafeZoneStartUp.h"
#include "EAirCraftBehavior.h"
#include "EFortCustomMovement.h"
#include "FortAircraftTimingRuntime.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CurveTable.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Volume.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ScriptMacros.h"

namespace FortAthenaMatchFlowPrivate
{
    // Asset paths (data: /mnt/vibrato/fortnite-extracted-full/FortniteGame/Content).
    const TCHAR* const PlaylistPath = TEXT("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
    const TCHAR* const GameDataPath = TEXT("/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData");
    const TCHAR* const MapInfoClassPath = TEXT("/Game/Athena/Balance/MapInfos/DefaultMapInfo.DefaultMapInfo_C");
    // Athena_GameMode Blueprint defaults (data: Athena/Athena_GameMode, SafeZoneIndicatorClass / GE_OutsideSafeZone).
    const TCHAR* const SafeZoneIndicatorClassPath = TEXT("/Game/Athena/SafeZone/SafeZoneIndicator.SafeZoneIndicator_C");
    const TCHAR* const OutsideSafeZoneGEPath = TEXT("/Game/Athena/SafeZone/GE_OutsideSafeZoneDamage.GE_OutsideSafeZoneDamage_C");

    const float DriverInterval = 0.05f;

    struct FBusPlayerState
    {
        bool bLeftAircraft = false;         // aircraft component byte +0xf9 (R5 4.2); no field exists in our class.
        bool bPendingExit = false;          // set by the exit path just before RestartPlayer
        bool bJump = false;                 // exit was the jump RPC (false: forced exit at drop end)
        FRotator ExitRotation = FRotator::ZeroRotator;
        TWeakObjectPtr<AFortAthenaAircraft> ExitAircraft;
    };

    struct FAircraftRunState
    {
        bool bEnteredDropZone = false;
        bool bExitedDropZone = false;
        bool bFlightEnded = false;
        float LastTimestampReplication = -1.0f;
    };

    struct FSkydiverState
    {
        bool bLanded = false;
        float LandedTime = -1.0f;
    };

    struct FPawnStormState
    {
        FActiveGameplayEffectHandle Handle;
        float LastStandInDamageTime = -1.0f;
    };

    struct FMatchFlowState
    {
        bool bActive = false;
        bool bSetupDone = false;
        bool bWarmupStarted = false;     // own flag: GamePhase may already be Warmup from the G6 stand-in
        bool bEarlyCountdownStarted = false;  // game mode +0xd11 bit 2 (R1 4)
        // Game mode +0xd10 bits (R1 1); not reflected, so kept here.
        bool bSkipWarmup = false;
        bool bPauseWarmup = false;
        bool bSkipAircraft = false;
        bool bPauseAircraft = false;
        bool bSkipSafeZones = false;
        int32 RequiredPlayers = 80;
        float EarlyRequiredPlayerPercent = 0.0f;  // game mode +0xf8c
        int32 PlayersAtWarmupEnd = 0;             // game mode +0xd78
        int32 AircraftsExitedDropZone = 0;        // game mode +0xd90
        float WarmupStartWorldTime = 0.0f;        // game mode +0x10a0
        bool bFirstAircraftStart = true;          // game mode +0xf80 == 0
        float MatchTime = 0.0f;                   // game mode +0x10cc
        float LastDriverTime = -1.0f;
        float LastStatusTime = -1.0f;
        float SafeZoneTimeScale = 1.0f;           // stand-in test option
        bool bStormCentered = false;              // stand-in test option
        TWeakObjectPtr<AFortGameModeAthena> GameMode;
        TWeakObjectPtr<UFortPlaylistAthena> Playlist;
        TWeakObjectPtr<UCurveTable> GameData;
        FTimerHandle DriverHandle;
        TMap<TWeakObjectPtr<AController>, FBusPlayerState> Players;
        TMap<TWeakObjectPtr<AFortAthenaAircraft>, FAircraftRunState> AircraftRuns;
        TMap<TWeakObjectPtr<APawn>, FSkydiverState> Skydivers;
        TMap<TWeakObjectPtr<APawn>, FPawnStormState> PawnStorm;
        TSet<FString> LoggedOnce;
    };

    FMatchFlowState& State()
    {
        static FMatchFlowState Instance;
        return Instance;
    }

    bool OriginalClientMode()
    {
        return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
    }

    void LogOnce(const FString& Key, const FString& Message)
    {
        if (State().LoggedOnce.Contains(Key))
            return;
        State().LoggedOnce.Add(Key);
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: %s"), *Message);
    }

    const TCHAR* PhaseName(EAthenaGamePhase Phase)
    {
        switch (Phase)
        {
        case EAthenaGamePhase::None: return TEXT("None");
        case EAthenaGamePhase::Setup: return TEXT("Setup");
        case EAthenaGamePhase::Warmup: return TEXT("Warmup");
        case EAthenaGamePhase::Aircraft: return TEXT("Aircraft");
        case EAthenaGamePhase::SafeZones: return TEXT("SafeZones");
        case EAthenaGamePhase::EndGame: return TEXT("EndGame");
        default: return TEXT("Unknown");
        }
    }

    // Calls a (possibly protected) reflected function by name, as the engine's OnRep dispatch does.
    void CallUFunction(UObject* Object, const TCHAR* Name, void* Params = nullptr)
    {
        if (!Object)
            return;
        if (UFunction* Function = Object->FindFunction(FName(Name)))
            Object->ProcessEvent(Function, Params);
        else
            LogOnce(FString::Printf(TEXT("nofunc:%s"), Name), FString::Printf(TEXT("function %s not found on %s"), Name, *Object->GetClass()->GetName()));
    }

    AFortGameModeAthena* GetGameMode() { return State().GameMode.Get(); }

    AFortGameStateAthena* GetGameState(AFortGameModeAthena* GameMode)
    {
        return GameMode ? GameMode->GetGameState<AFortGameStateAthena>() : nullptr;
    }

    UFortPlaylistAthena* GetPlaylist(AFortGameStateAthena* GameState)
    {
        return GameState ? GameState->GetCurrentPlaylistAthena() : nullptr;
    }

    // GameState virtual slot +0x630 (server world time, inferred in R1 4).
    float Now(AFortGameStateAthena* GameState)
    {
        return GameState ? GameState->GetServerWorldTimeSeconds() : 0.0f;
    }

    ESafeZoneStartUp SafeZoneStartUp(AFortGameStateAthena* GameState)
    {
        const UFortPlaylistAthena* Playlist = GetPlaylist(GameState);
        return Playlist ? Playlist->SafeZoneStartUp : ESafeZoneStartUp::UseDefaultGameBehavior;
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Safe zone table (R2 1.1). The original caches every curve per level; reading the curves directly is equivalent.
    // ---------------------------------------------------------------------------------------------------------------
    int32 SafeZonePhaseCount(const AFortAthenaMapInfo* MapInfo)  // 0x1418edcb0
    {
        return MapInfo ? FMath::RoundToInt(MapInfo->SafeZoneDefinition.Count.GetValueAtLevel(0.0f)) : 0;
    }

    float SafeZoneValue(const AFortAthenaMapInfo* MapInfo, const FScalableFloat& Curve, int32 Phase)
    {
        // Out of range reads as 0 (R2 3 step 4, 6 step 4: "0 when missing").
        if (!MapInfo || Phase < 0 || Phase >= SafeZonePhaseCount(MapInfo))
            return 0.0f;
        return Curve.GetValueAtLevel(static_cast<float>(Phase));
    }

    float ScaledTime(float Seconds)
    {
        // stand-in: -FortniteSafeZoneTimeScale=<f> shortens storm wait/shrink/start delays for test runs. 1 = original.
        return Seconds * State().SafeZoneTimeScale;
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Phase setter, AFortGameStateAthena 0x1419ae750 (R1 3.1), including the server-relevant steps of the phase-change
    // handler 0x14199ac00 (R5 3), because OnRep_GamePhase is a reflection stub in our server.
    // ---------------------------------------------------------------------------------------------------------------
    void PlacePlayersInAircraft(AFortGameStateAthena* GameState);

    void SetGamePhase(AFortGameStateAthena* GameState, EAthenaGamePhase NewPhase)
    {
        if (!GameState || !GameState->HasAuthority() || GameState->GamePhase == NewPhase)
            return;
        const EAthenaGamePhase OldPhase = GameState->GamePhase;
        GameState->GamePhase = NewPhase;
        // Steps 3 (0x14069ac00 notification for SafeZones/EndGame) and 4 (net driver connection timeout from
        // InGameConnectionTimeOutPVP) are not ported.
        LogOnce(TEXT("setter-unported"), TEXT("phase setter: notification 0x14069ac00 and net driver timeout update not ported"));

        // stand-in: GamePhaseStep is not replicated and its writer is unread; keep the server copy plausible (G6 precedent).
        switch (NewPhase)
        {
        case EAthenaGamePhase::Warmup: GameState->GamePhaseStep = EAthenaGamePhaseStep::Warmup; break;
        case EAthenaGamePhase::Aircraft: GameState->GamePhaseStep = EAthenaGamePhaseStep::BusLocked; break;
        case EAthenaGamePhase::SafeZones: GameState->GamePhaseStep = EAthenaGamePhaseStep::StormForming; break;
        case EAthenaGamePhase::EndGame: GameState->GamePhaseStep = EAthenaGamePhaseStep::EndGame; break;
        default: break;
        }

        // Step 5: the phase-change handler (OnRep_GamePhase). Our generated body is a stub; call it for fidelity and do
        // the server-side effects here: step 8 (PlacePlayersInAircraft), step 9 (UtcTimeStartedMatch), step 13
        // (GamePhaseChanged). Steps 4-7 (level streaming, LTM loading loops, sound mix) are not ported.
        struct { EAthenaGamePhase OldGamePhase; } RepParams{ OldPhase };
        CallUFunction(GameState, TEXT("OnRep_GamePhase"), &RepParams);
        if (NewPhase == EAthenaGamePhase::Aircraft)
            PlacePlayersInAircraft(GameState);
        if (NewPhase == EAthenaGamePhase::Warmup)
            GameState->UtcTimeStartedMatch = FDateTime::UtcNow();
        GameState->GamePhaseChanged.Broadcast(NewPhase);

        // Step 6: GameState slot +0x5e8 (inferred: force net update).
        GameState->ForceNetUpdate();
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: Set AthenaGamePhase to [%s] (from %s) at %.2f"), PhaseName(NewPhase), PhaseName(OldPhase), Now(GameState));
    }

    // ---------------------------------------------------------------------------------------------------------------
    // MapInfo validity 0x1418f1550 (R6 2.2)
    // ---------------------------------------------------------------------------------------------------------------
    bool IsMapInfoValid(const AFortAthenaMapInfo* MapInfo)
    {
        if (!MapInfo)
            return false;
        if (!MapInfo->AircraftClass || !MapInfo->AircraftClass->IsChildOf(AFortAthenaAircraft::StaticClass()))
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: IsMapInfoValid: AircraftClass missing"));
            return false;
        }
        if (MapInfo->AircraftSpeed.GetValueAtLevel(0.0f) <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: IsMapInfoValid: AircraftSpeed <= 0"));
            return false;
        }
        const FBox2D& Spawn = MapInfo->AircraftSpawnZone;
        if (!(Spawn.Min.X < Spawn.Max.X && Spawn.Min.Y < Spawn.Max.Y))
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: IsMapInfoValid: AircraftSpawnZone invalid"));
            return false;
        }
        if (MapInfo->AircraftDropVolume)
            return true;
        const FBox2D& Drop = MapInfo->AircraftDropZone;
        if (!(Drop.Min.X < Drop.Max.X && Drop.Min.Y < Drop.Max.Y) ||
            !(Drop.Min.X > Spawn.Min.X && Drop.Min.Y > Spawn.Min.Y && Drop.Max.X < Spawn.Max.X && Drop.Max.Y < Spawn.Max.Y))
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: IsMapInfoValid: AircraftDropZone invalid or not inside spawn zone"));
            return false;
        }
        return true;
    }

    // Distance along Dir from a point inside Box to the box edge.
    float ExitDistance(const FBox2D& Box, const FVector2D& From, const FVector2D& Dir)
    {
        float Best = TNumericLimits<float>::Max();
        if (FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER)
            Best = FMath::Min(Best, ((Dir.X > 0.0f ? Box.Max.X : Box.Min.X) - From.X) / Dir.X);
        if (FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER)
            Best = FMath::Min(Best, ((Dir.Y > 0.0f ? Box.Max.Y : Box.Min.Y) - From.Y) / Dir.Y);
        return FMath::Max(0.0f, Best);
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Flight path: MapInfo 0x1418ef640 -> InitializeFlightPath 0x1418e4610 -> 0x1418f8580 (R5 1, R6 5).
    // ---------------------------------------------------------------------------------------------------------------
    bool InitializeFlightPaths(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState, AFortAthenaMapInfo* MapInfo)
    {
        const UFortPlaylistAthena* Playlist = GetPlaylist(GameState);
        const EAirCraftBehavior Behavior = Playlist ? Playlist->AirCraftBehavior : EAirCraftBehavior::Default;
        GameState->AirCraftBehavior = Behavior;
        if (Behavior == EAirCraftBehavior::OpposingAirCraftForEachTeam || Behavior == EAirCraftBehavior::FlyTowardFirstCircleCenter)
            LogOnce(TEXT("behavior"), FString::Printf(TEXT("AirCraftBehavior %d not ported; using the drop-zone path of Default"), (int32)Behavior));

        // Step 2: boxes offset by the MapInfo actor location.
        const FVector MapOrigin = MapInfo->GetActorLocation();
        const FVector2D Offset(MapOrigin.X, MapOrigin.Y);
        const FBox2D SpawnBox(MapInfo->AircraftSpawnZone.Min + Offset, MapInfo->AircraftSpawnZone.Max + Offset);
        FBox2D DropBox(MapInfo->AircraftDropZone.Min + Offset, MapInfo->AircraftDropZone.Max + Offset);
        if (MapInfo->AircraftDropVolume)
        {
            const FBox Bounds = MapInfo->AircraftDropVolume->GetComponentsBoundingBox(true);
            DropBox = FBox2D(FVector2D(Bounds.Min.X, Bounds.Min.Y), FVector2D(Bounds.Max.X, Bounds.Max.Y));
        }
        const float Height = MapOrigin.Z + MapInfo->AircraftHeight.GetValueAtLevel(0.0f);
        const float Speed = MapInfo->AircraftSpeed.GetValueAtLevel(0.0f);

        // Steps 4-6.
        GameState->TeamFlightPaths.Reset();
        const FVector2D Center = DropBox.GetCenter();
        GameState->DropZoneCenter = Center;

        // Step 7: the random direction. stand-in: the exact rand()-based angle/offset formulas were not followed term by
        // term (R5 1.1 step 7); use a uniform yaw through the drop-zone centre, or -FortniteFlightYaw=<deg> for repeatable
        // runs. AircraftDeviationAngle is not applied.
        float YawDegrees = FMath::FRandRange(0.0f, 360.0f);
        FParse::Value(FCommandLine::Get(), TEXT("FortniteFlightYaw="), YawDegrees);
        const FVector2D Dir(FMath::Cos(FMath::DegreesToRadians(YawDegrees)), FMath::Sin(FMath::DegreesToRadians(YawDegrees)));

        // InitializeFlightPath steps 3-4: project the line onto the spawn box and the drop box (line-onto-box 0x1418f5b60
        // is not read; a ray-box exit through the centre is the plain reading of "project").
        const float SpawnBack = ExitDistance(SpawnBox, Center, -Dir);
        const float SpawnForward = ExitDistance(SpawnBox, Center, Dir);
        const float DropBack = ExitDistance(DropBox, Center, -Dir);
        const float DropForward = ExitDistance(DropBox, Center, Dir);
        if (SpawnBack <= 0.0f || SpawnForward <= 0.0f || Speed <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: InitializeFlightPath: Failed to project path onto spawn zone."));
            return false;
        }
        const FVector2D Start2D = Center - Dir * SpawnBack;

        // 0x1418f8580 fill (R6 5): start location + height, yaw, speed, three times (distance / speed).
        FAircraftFlightInfo Info{};
        Info.FlightStartLocation = FVector(Start2D.X, Start2D.Y, Height);
        Info.FlightStartRotation = FRotator(0.0f, YawDegrees, 0.0f);
        Info.FlightSpeed = Speed;
        Info.TimeTillFlightEnd = (SpawnBack + SpawnForward) / Speed;
        Info.TimeTillDropStart = FMath::Max(0.0f, SpawnBack - DropBack) / Speed;
        Info.TimeTillDropEnd = FMath::Max(Info.TimeTillDropStart, (SpawnBack + DropForward) / Speed);
        // Global 0x1478c2420 (move start back for AircraftDesiredDoorOpenTime) is off by default; not ported.

        // Step 8: one aircraft for Default behavior.
        MapInfo->FlightInfos.Reset();
        MapInfo->FlightInfos.Add(Info);
        GameState->TeamFlightPaths.Add(Info);
        // Step 9: fewer than 2 aircraft -> mid line and its midpoint.
        GameState->FlightPathMidLine = Info;
        GameState->FlightPathSelectionCenter = Start2D + Dir * ((SpawnBack + SpawnForward) * 0.5f);
        GameMode->bFlightPathInitialized = true;

        UE_LOG(LogTemp, Display, TEXT("MatchFlow: flight path start=%s yaw=%.1f speed=%.0f dropStart=%.1fs dropEnd=%.1fs end=%.1fs"),
            *Info.FlightStartLocation.ToString(), YawDegrees, Speed, Info.TimeTillDropStart, Info.TimeTillDropEnd, Info.TimeTillFlightEnd);
        return true;
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Storm centres: MapInfo 0x1418e7000/0x1418e7080 -> PickSafeZoneLocation 0x1418f4530 (R5 2).
    // ---------------------------------------------------------------------------------------------------------------
    FVector PickSafeZoneLocation(int32 Index, const FVector& PreviousCenter, float MaxRadius, float RejectRadius, float RejectOuterDistance,
                                 float ForceDistanceMin, float ForceDistanceMax)
    {
        const bool bForced = ForceDistanceMax > 0.0f && ForceDistanceMax >= ForceDistanceMin;
        float PickRadius = MaxRadius;
        if (!bForced && RejectOuterDistance >= 0.0f)
        {
            PickRadius = MaxRadius - RejectOuterDistance;
            if (PickRadius <= RejectRadius)
            {
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: PickSafeZoneLocation[%d]: RejectOuterDistance will make acceptable radius too small. Centering."), Index);
                PickRadius = 0.0f;
            }
        }
        if (MaxRadius <= 0.0f)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: PickSafeZoneLocation[%d]: Invalid max radius. Returning root location."), Index);
            return PreviousCenter;
        }
        // stand-in: FMath::FRand replaces the C runtime rand(); reject rectangles and SafeZoneVolumeDefinitions are empty
        // on a spawned MapInfo (R6 2.2), so only the radius tests remain.
        for (int32 Try = 0; Try < 10000; ++Try)
        {
            FVector2D Offset2D;
            do { Offset2D = FVector2D(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f)); } while (Offset2D.SizeSquared() > 1.0f);
            if (bForced)
                Offset2D = Offset2D.GetSafeNormal() * FMath::FRandRange(ForceDistanceMin, ForceDistanceMax);
            else
                Offset2D *= PickRadius;
            const float Distance = Offset2D.Size();
            if (Distance > MaxRadius)
                continue;
            if (!bForced && RejectRadius > 0.0f && Distance < RejectRadius)
                continue;
            const FVector Location(PreviousCenter.X + Offset2D.X, PreviousCenter.Y + Offset2D.Y, PreviousCenter.Z);
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: PickSafeZoneLocation[%d]: Setting Location: %0.2f, %0.2f, %0.2f"), Index, Location.X, Location.Y, Location.Z);
            return Location;
        }
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: PickSafeZoneLocation[%d]: Failed to find valid location using rejection."), Index);
        return PreviousCenter;
    }

    void InitializeSafeZoneLocations(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState, AFortAthenaMapInfo* MapInfo)
    {
        const FFortSafeZoneDefinition& Def = MapInfo->SafeZoneDefinition;
        const int32 Count = SafeZonePhaseCount(MapInfo);
        GameMode->SafeZoneLocations.Reset();
        GameMode->SafeZoneLocations.SetNumZeroed(FMath::Max(Count, 0));
        if (Count <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: storm centres: SafeZoneDefinition.Count is %d"), Count);
            return;
        }
        // Root: inferred to be the MapInfo location (the argument was not named in R5 2.1). Element 0 takes X/Y from
        // FlightPathSelectionCenter, which is why the flight path is set up first.
        FVector Root = MapInfo->GetActorLocation();
        GameMode->SafeZoneLocations[0] = FVector(GameState->FlightPathSelectionCenter.X, GameState->FlightPathSelectionCenter.Y, Root.Z);
        for (int32 Phase = 1; Phase < Count; ++Phase)
        {
            const float Radius = SafeZoneValue(MapInfo, Def.Radius, Phase);
            if (State().bStormCentered || Radius <= 0.0f)
            {
                // Radius <= 0: bCenterAll branch of 0x1418e7080 (centre on root). bStormCentered is a stand-in test option.
                GameMode->SafeZoneLocations[Phase] = GameMode->SafeZoneLocations[0];
                continue;
            }
            // Parameter block mapping (R5 2.1 says it was matched by the log line, not traced): max radius is taken as
            // the previous radius minus this radius, so each circle lies inside the one before (inferred).
            const float PreviousRadius = SafeZoneValue(MapInfo, Def.Radius, Phase - 1);
            GameMode->SafeZoneLocations[Phase] = PickSafeZoneLocation(Phase, GameMode->SafeZoneLocations[Phase - 1],
                FMath::Max(0.0f, PreviousRadius - Radius), SafeZoneValue(MapInfo, Def.RejectRadius, Phase),
                SafeZoneValue(MapInfo, Def.RejectOuterDistance, Phase), SafeZoneValue(MapInfo, Def.ForceDistanceMin, Phase),
                SafeZoneValue(MapInfo, Def.ForceDistanceMax, Phase));
        }
        GameMode->bSafeZoneLocationsInitialized = true;
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: %d storm centres picked, first %s"), Count, *GameMode->SafeZoneLocations[0].ToString());
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Setup: the playlist trigger (R6 1.3 option 2) and OnPlaylistDataLoaded 0x141955570 subset (R1 6.1, R5 1-2).
    // ---------------------------------------------------------------------------------------------------------------
    void FillSafeZoneDefinition(AFortAthenaMapInfo* MapInfo, UCurveTable* GameData)
    {
        // R6 2.1/2.2: the twelve SafeZoneDefinition curves are instance data of DefaultMapInfo_2 in Apollo_Terrain.umap.
        FFortSafeZoneDefinition& Def = MapInfo->SafeZoneDefinition;
        Def.Count.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.Count")), GameData);
        Def.Radius.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.Radius")), GameData);
        Def.ForceDistanceMin.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.ForceDistanceMin")), GameData);
        Def.ForceDistanceMax.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.ForceDistanceMax")), GameData);
        Def.RejectRadius.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.RejectRadius")), GameData);
        Def.RejectOuterDistance.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.RejectOuterDistance")), GameData);
        Def.WaitTime.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.WaitTime")), GameData);
        Def.ShrinkTime.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.ShrinkTime")), GameData);
        Def.MegaStormGridCellThickness.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.MegaStormGridThickness")), GameData);
        Def.PlayerCapSolo.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.PlayerCap.Solo")), GameData);
        Def.PlayerCapDuo.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.PlayerCap.Duo")), GameData);
        Def.PlayerCapSquad.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.PlayerCap.Squad")), GameData);
        // SafeZoneVolumeDefinitions, AircraftDropVolume and SafeZoneStorm stay empty (R6 2.2 items 2-4).
    }

    bool SetupPlaylistAndMap(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
    {
        FMatchFlowState& S = State();
        UWorld* World = GameMode->GetWorld();

        // Playlist: write CurrentPlaylistInfo (BasePlaylist, replication key, dirty) and run OnRep_CurrentPlaylistInfo on
        // the server (R6 1.3). The chain below OnRep is not ported, so its game mode step is done directly here.
        UFortPlaylistAthena* Playlist = LoadObject<UFortPlaylistAthena>(nullptr, PlaylistPath);
        if (!Playlist)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: playlist %s did not load; match flow not started"), PlaylistPath);
            return false;
        }
        S.Playlist = Playlist;
        if (FParse::Param(FCommandLine::Get(), TEXT("FortniteLootDataProbe")))
            FortAthenaLootData::Initialize(GameMode, Playlist);
        GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
        GameState->CurrentPlaylistInfo.OverridePlaylist = nullptr;
        GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
        GameState->CurrentPlaylistInfo.MarkArrayDirty();
        GameState->CurrentPlaylistId = Playlist->PlaylistId;
        CallUFunction(GameState, TEXT("OnRep_CurrentPlaylistInfo"));
        // 0x1419b47b0 (playlist values into the GameState), first statement only: CachedSafeZoneStartUp.
        GameState->CachedSafeZoneStartUp = Playlist->SafeZoneStartUp;
        LogOnce(TEXT("playlist-copy"), TEXT("0x1419b47b0 playlist-to-GameState copy ported only for CachedSafeZoneStartUp; loot preload 0x1419a0870 not ported"));
        // HandleMatchHasStarted (D2 3 step 3): playlist may skip warmup / aircraft.
        if (Playlist->bSkipWarmup) { S.bSkipWarmup = true; UE_LOG(LogTemp, Display, TEXT("MatchFlow: Skipping Warmup due to playlist settings")); }
        if (Playlist->bSkipAircraft) { S.bSkipAircraft = true; UE_LOG(LogTemp, Display, TEXT("MatchFlow: Skipping Aircraft due to playlist settings")); }
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: playlist %s (id %d) set on the GameState, SafeZoneStartUp=%d AirCraftBehavior=%d"),
            *Playlist->GetName(), Playlist->PlaylistId, (int32)Playlist->SafeZoneStartUp, (int32)Playlist->AirCraftBehavior);

        UCurveTable* GameData = LoadObject<UCurveTable>(nullptr, GameDataPath);
        if (!GameData)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: %s did not load; scalable floats fall back to their plain values"), GameDataPath);
        }
        S.GameData = GameData;

        // MapInfo: spawn DefaultMapInfo_C and fill the instance data the map would carry (R6 2.2).
        UClass* MapInfoClass = LoadClass<AFortAthenaMapInfo>(nullptr, MapInfoClassPath);
        if (!MapInfoClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: %s did not load"), MapInfoClassPath);
            return false;
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AFortAthenaMapInfo* MapInfo = World->SpawnActor<AFortAthenaMapInfo>(MapInfoClass, FTransform::Identity, Params);
        if (!MapInfo)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: DefaultMapInfo_C spawn failed"));
            return false;
        }
        // stand-in: on Apollo_Terrain the MapInfo is a level actor; a spawned one must be forced to replicate.
        MapInfo->SetReplicates(true);
        MapInfo->bAlwaysRelevant = true;
        if (GameData)
            FillSafeZoneDefinition(MapInfo, GameData);
        // stand-in: -FortniteDropZoneExtent=<N> shrinks the drop zone box to +-N so bus exits land on the test floor of
        // FortniteEmptyDedicated (default box is +-80000, the test floor about +-50000).
        float DropExtent = 0.0f;
        if (FParse::Value(FCommandLine::Get(), TEXT("FortniteDropZoneExtent="), DropExtent) && DropExtent > 0.0f)
        {
            MapInfo->AircraftDropZone = FBox2D(FVector2D(-DropExtent, -DropExtent), FVector2D(DropExtent, DropExtent));
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: stand-in drop zone +-%.0f"), DropExtent);
        }
        GameState->MapInfo = MapInfo;
        CallUFunction(GameState, TEXT("OnRep_MapInfo"));
        GameMode->bMapInfoInitialized = true;  // inferred: the reflected flag behind IsMapInfoInitialized
        GameState->ForceNetUpdate();
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: spawned %s replicates=%d alwaysRelevant=%d phases=%d speed=%.0f height=%.0f"),
            *MapInfo->GetName(), MapInfo->GetIsReplicated() ? 1 : 0, MapInfo->bAlwaysRelevant ? 1 : 0, SafeZonePhaseCount(MapInfo),
            MapInfo->AircraftSpeed.GetValueAtLevel(0.0f), MapInfo->AircraftHeight.GetValueAtLevel(0.0f));

        // OnPlaylistDataLoaded: flight path once, then storm centres (order matters, R5 2.1 step 2).
        if (!GameMode->bFlightPathInitialized && IsMapInfoValid(MapInfo))
            InitializeFlightPaths(GameMode, GameState, MapInfo);
        if (!GameMode->bSafeZoneLocationsInitialized)
            InitializeSafeZoneLocations(GameMode, GameState, MapInfo);

        // Blueprint class defaults the native game mode lacks (data: Athena_GameMode).
        if (!GameMode->SafeZoneIndicatorClass)
            GameMode->SafeZoneIndicatorClass = LoadClass<AFortSafeZoneIndicator>(nullptr, SafeZoneIndicatorClassPath);
        if (!GameMode->GE_OutsideSafeZone)
            GameMode->GE_OutsideSafeZone = LoadClass<UGameplayEffect>(nullptr, OutsideSafeZoneGEPath);
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: SafeZoneIndicatorClass=%s GE_OutsideSafeZone=%s"),
            GameMode->SafeZoneIndicatorClass ? *GameMode->SafeZoneIndicatorClass->GetName() : TEXT("none"),
            GameMode->GE_OutsideSafeZone ? *GameMode->GE_OutsideSafeZone->GetName() : TEXT("none"));

        // Initial time sentinels and playlist defaults come from FortOriginalDefaults before Blueprint loading.
        return true;
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Ready-player counter 0x14193af00 (R1 4.1)
    // ---------------------------------------------------------------------------------------------------------------
    int32 CountReadyPlayers(UWorld* World)
    {
        int32 Count = 0;
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(It->Get());
            if (PC && PC->PlayerState && !PC->PlayerState->IsSpectator() && PC->bReadyToStartMatch)
                ++Count;
            // bExcludeFlagged (controller byte +0x2d69) is not named; every call site here passes false.
        }
        return Count;
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Aircraft spawn 0x1418d4da0 / pre-spawn 0x141967bd0 (R1 6.1) and flight start 0x1418d5590 (R1 6.2)
    // ---------------------------------------------------------------------------------------------------------------
    int32 AircraftCount(AFortGameStateAthena* GameState)
    {
        const UFortPlaylistAthena* Playlist = GetPlaylist(GameState);
        const EAirCraftBehavior Behavior = Playlist ? Playlist->AirCraftBehavior : EAirCraftBehavior::Default;
        if (Behavior == EAirCraftBehavior::NoAircraft)
            return 0;
        // OpposingAirCraftForEachTeam uses TeamCount; not ported (one flight path is built).
        return 1;
    }

    AFortAthenaAircraft* SpawnAircraft(UWorld* World, AFortGameStateAthena* GameState, int32 Index)
    {
        AFortAthenaMapInfo* MapInfo = GameState->MapInfo;
        if (!IsMapInfoValid(MapInfo) || !MapInfo->FlightInfos.IsValidIndex(Index))
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: aircraft %d: MapInfo invalid or no flight info"), Index);
            return nullptr;
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;  // collision override 1
        Params.bNoFail = true;  // inferred part of flags |= 6
        AFortAthenaAircraft* Aircraft = World->SpawnActor<AFortAthenaAircraft>(MapInfo->AircraftClass, FTransform::Identity, Params);
        if (!Aircraft)
            return nullptr;
        Aircraft->AircraftIndex = Index;
        Aircraft->FlightStartTime = -1.0f;
        Aircraft->FlightEndTime = -1.0f;
        Aircraft->DropStartTime = -1.0f;
        Aircraft->DropEndTime = -1.0f;
        Aircraft->FlightInfo = MapInfo->FlightInfos[Index];
        Aircraft->SetActorLocationAndRotation(Aircraft->FlightInfo.FlightStartLocation, Aircraft->FlightInfo.FlightStartRotation);
        // stand-in: force replication (bare generated constructor; the Blueprint may already set it).
        Aircraft->SetReplicates(true);
        Aircraft->bAlwaysRelevant = true;
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: spawned aircraft %s index %d at %s replicates=%d"), *Aircraft->GetName(), Index,
            *Aircraft->GetActorLocation().ToString(), Aircraft->GetIsReplicated() ? 1 : 0);
        return Aircraft;
    }

    void StartFlight(AFortAthenaAircraft* Aircraft, float Time)
    {
        if (Aircraft->FlightStartTime >= 0.0f)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: StartFlightPath: Flight was already started."));
            return;
        }
        Aircraft->FlightStartTime = Time;
        Aircraft->ReplicatedFlightTimestamp = Time;
        Aircraft->FlightElapsedTime = 0.0f;
        Aircraft->FlightEndTime = Time + Aircraft->FlightInfo.TimeTillFlightEnd;
        Aircraft->DropStartTime = Time + Aircraft->FlightInfo.TimeTillDropStart;
        Aircraft->DropEndTime = Time + Aircraft->FlightInfo.TimeTillDropEnd;
        Aircraft->ForceNetUpdate();
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: bus flight started at %.2f: dropStart %.2f dropEnd %.2f end %.2f"),
            Time, Aircraft->DropStartTime, Aircraft->DropEndTime, Aircraft->FlightEndTime);
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Safe zones: IsSafeZoneAllowed 0x141971930, StartSafeZonesPhase 0x141971aa0 (R2 2-3)
    // ---------------------------------------------------------------------------------------------------------------
    void SetShrinkTimes(AFortSafeZoneIndicator* Indicator, AFortAthenaMapInfo* MapInfo, int32 Phase, float NowTime)
    {
        const float Wait = SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.WaitTime, Phase);
        if (Wait >= 0.0f)
        {
            Indicator->bSafezoneEventDriven = false;
            Indicator->SafeZoneStartShrinkTime = NowTime + ScaledTime(Wait);
            Indicator->SafeZoneFinishShrinkTime = Indicator->SafeZoneStartShrinkTime + ScaledTime(SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.ShrinkTime, Phase));
        }
        else
        {
            Indicator->bSafezoneEventDriven = true;  // inferred from the branch structure (R2 3 step 4)
        }
    }

    bool StartSafeZonesPhase(AFortGameModeAthena* GameMode, bool bForce)
    {
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        if (!GameState || !GameState->MapInfo)
            return false;
        if (State().bSkipSafeZones)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: Skipping SafeZone phase: bSkipSafeZones == true"));
            return false;
        }
        if (GameState->GamePhase == EAthenaGamePhase::SafeZones && !bForce)
            return true;
        SetGamePhase(GameState, EAthenaGamePhase::SafeZones);
        if (AFortSafeZoneIndicator* Indicator = GameMode->SafeZoneIndicator)
        {
            SetShrinkTimes(Indicator, GameState->MapInfo, GameMode->SafeZonePhase, Now(GameState));
            Indicator->ForceNetUpdate();
        }
        // Step 5: GameState 0x1419964d0 when 0x14199e430 is false; real start not resolved (R2 3).
        LogOnce(TEXT("sz-step5"), TEXT("StartSafeZonesPhase step 5 (GameState 0x1419964d0) not ported"));
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: StartSafeZonesPhase: storm starts at %.2f"), GameState->SafeZonesStartTime);
        return true;
    }

    bool IsSafeZoneAllowed(AFortGameModeAthena* GameMode, bool bForce, bool bStartPhase)
    {
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        if (!GameState || !GameState->MapInfo || SafeZonePhaseCount(GameState->MapInfo) <= 1)
            return false;
        if (State().bSkipSafeZones)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: Skipping SafeZone phase: bSkipSafeZones == true"));
            return false;
        }
        const ESafeZoneStartUp StartUp = SafeZoneStartUp(GameState);
        const EAthenaGamePhase Phase = GameState->GamePhase;
        if (!bForce &&
            (Phase == EAthenaGamePhase::SafeZones ||
             (Phase == EAthenaGamePhase::Warmup && StartUp == ESafeZoneStartUp::StartsWithWarmUp) ||
             (Phase == EAthenaGamePhase::Aircraft && (StartUp == ESafeZoneStartUp::StartsWithWarmUp || StartUp == ESafeZoneStartUp::StartsWithAirCraft))))
        {
            return true;
        }
        GameMode->bSafeZoneActive = false;
        GameState->SafeZonesStartTime = Now(GameState) + ScaledTime(GameState->MapInfo->SafeZoneStartDelay.GetValueAtLevel(0.0f));
        GameState->ForceNetUpdate();
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: safe zones armed, SafeZonesStartTime=%.2f"), GameState->SafeZonesStartTime);
        if (bStartPhase)
            return StartSafeZonesPhase(GameMode, bForce);
        return true;
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Bus placement: PlacePlayersInAircraft 0x1419a8ec0, EnterAircraft 0x141ccfa40 (R5 4)
    // ---------------------------------------------------------------------------------------------------------------
    UFortControllerComponent_Aircraft* AircraftComponent(AController* Controller)
    {
        return Controller ? Controller->FindComponentByClass<UFortControllerComponent_Aircraft>() : nullptr;
    }

    void EnterAircraft(UFortControllerComponent_Aircraft* Component, AFortAthenaAircraft* Aircraft)
    {
        FBusPlayerState& Player = State().Players.FindOrAdd(TWeakObjectPtr<AController>(Cast<AController>(Component->GetOwner())));
        if (Player.bLeftAircraft)
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: EnterAircraft: [%s] is attempting to enter aircraft after having already exited."), *Component->GetOwner()->GetName());
        if (Component->NativeRecoveredBool_0xf8)
            return;
        // Read part: controller calls 0x14210e280 / 0x142123bf0 and HUD-side reference, not ported. The rest of the
        // function is unread (R5 4.2); +0xf8 and CurrentAircraft are written there (inferred).
        Component->NativeRecoveredBool_0xf8 = true;
        Component->CurrentAircraft = Aircraft;
        if (AController* Controller = Cast<AController>(Component->GetOwner()))
        {
            // D5 design (ServerSetInAircraft): bInAircraft on the PlayerState (inferred to be set at entry).
            if (AFortPlayerStateZone* PlayerState = Cast<AFortPlayerStateZone>(Controller->PlayerState))
            {
                PlayerState->bInAircraft = 1;
                PlayerState->ForceNetUpdate();
            }
        }
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: EnterAircraft: %s"), *Component->GetOwner()->GetName());
        LogOnce(TEXT("enter-unread"), TEXT("EnterAircraft: unread tail of 0x141ccfa40 (controller slots +0xbd8/+0xbe0) not ported; no ClientEnterAircraft sent (client runs its own placement)"));
    }

    void PlacePlayersInAircraft(AFortGameStateAthena* GameState)
    {
        // Step 1: mutator take-over (slot +0xb40) not ported; no mutators are spawned.
        if (GameState->Aircrafts.Num() == 0)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: PlacePlayersInAircraft early out. Aircrafts TArray is empty."));
            return;
        }
        for (AFortAthenaAircraft* Aircraft : GameState->Aircrafts)
        {
            if (!Aircraft)
            {
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: PlacePlayersInAircraft early out. Aircrafts TArray has invalid entries."));
                return;
            }
        }
        UWorld* World = GameState->GetWorld();
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(It->Get());
            if (!PC)
                continue;
            for (int32 Index = 0; Index < GameState->Aircrafts.Num(); ++Index)
            {
                // Controller index is 0 for every behavior but the per-team one.
                if (Index != 0)
                    continue;
                UFortControllerComponent_Aircraft* Component = AircraftComponent(PC);
                const FBusPlayerState* Player = State().Players.Find(TWeakObjectPtr<AController>(PC));
                if (Component && !Component->NativeRecoveredBool_0xf8 && !(Player && Player->bLeftAircraft))
                    EnterAircraft(Component, GameState->GetAircraft(Index));
                // 0x1419f5840 (per-controller view helper, inner calls unread) not ported.
            }
        }
        LogOnce(TEXT("place-view"), TEXT("PlacePlayersInAircraft: view helper 0x1419f5840 and quest triggers 0x1424a51b0 not ported"));
    }

    // ---------------------------------------------------------------------------------------------------------------
    // StartAircraftPhase 0x14196b640 (R1 6, R5 0) and the warmup-end helper 0x141975ef0 (R1 5)
    // ---------------------------------------------------------------------------------------------------------------
    void BindAircraftDelegates(AFortGameModeAthena* GameMode, AFortAthenaAircraft* Aircraft)
    {
        // +0x260 OnAircraftEnteredDropZone and +0x280 OnAircraftFlightEnded are UFUNCTIONs and are bound by name.
        // OnAircraftExitedDropZone (+0x270) is not a reflected function in our game mode: the driver calls the free
        // function below directly (see TickAircraft).
        FScriptDelegate Entered;
        Entered.BindUFunction(GameMode, FName(TEXT("OnAircraftEnteredDropZone")));
        Aircraft->NativeAircraftEnteredDropZone.AddUnique(Entered);
        FScriptDelegate Ended;
        Ended.BindUFunction(GameMode, FName(TEXT("OnAircraftFlightEnded")));
        Aircraft->NativeAircraftFlightEnded.AddUnique(Ended);
    }

    bool StartAircraftPhase(AFortGameModeAthena* GameMode, bool bSecond)
    {
        FMatchFlowState& S = State();
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        UWorld* World = GameMode->GetWorld();
        // Step 1: RemoveMissingPlayersFromSession 0x14195eb70 and recent players 0x141977980 are session bookkeeping the
        // client never sees; left out.
        if (S.bFirstAircraftStart)
            S.bFirstAircraftStart = false;
        // Step 2: wall-clock start time (+0x1090/+0x1098) is only used for analytics; left out.
        if (!GameState || S.bSkipAircraft)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: Skipping aircraft phase"));
            return false;
        }
        if (GameState->GamePhase > EAthenaGamePhase::Aircraft)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: Already past aircraft phase"));
            return false;
        }
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: Initiating aircraft phase"));
        if (GameState->GamePhase == EAthenaGamePhase::Aircraft)
            return true;
        UFortPlaylistAthena* Playlist = GetPlaylist(GameState);
        if (!Playlist)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: Playlist is Null!"));
            return false;
        }

        // Step 4.
        if (!bSecond && Playlist->AirCraftBehavior != EAirCraftBehavior::NoAircraft)
        {
            const int32 Count = AircraftCount(GameState);
            const float NowTime = Now(GameState);
            for (int32 Index = 0; Index < Count; ++Index)
            {
                AFortAthenaAircraft* Aircraft = GameMode->Aircrafts.IsValidIndex(Index) ? GameMode->Aircrafts[Index] : nullptr;
                if (!Aircraft)
                {
                    Aircraft = SpawnAircraft(World, GameState, Index);
                    if (!Aircraft)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("MatchFlow: Unable to spawn aircraft."));
                        return false;
                    }
                    if (GameMode->Aircrafts.IsValidIndex(Index)) GameMode->Aircrafts[Index] = Aircraft; else GameMode->Aircrafts.Add(Aircraft);
                }
                // Add to the GameState, 0x1419afc30: on the first append lock the bus and zero +0x1418.
                if (GameState->Aircrafts.Num() == 0)
                {
                    GameState->bAircraftIsLocked = 1;
                    GameState->AircraftUnlockPlayerCount = 0;
                }
                GameState->Aircrafts.Add(Aircraft);
                if (!S.bPauseAircraft)
                    StartFlight(Aircraft, NowTime);
                BindAircraftDelegates(GameMode, Aircraft);
                S.AircraftRuns.FindOrAdd(TWeakObjectPtr<AFortAthenaAircraft>(Aircraft));
            }
            GameState->AircraftStartTime = NowTime;
            // Analytics record and StartMatchAbandonProcess 0x14196d920 (924 lines, not read) left out.
            LogOnce(TEXT("abandon"), TEXT("StartAircraftPhase: analytics and StartMatchAbandonProcess 0x14196d920 not ported"));
        }

        FortAthenaHealth::BeginMatch(GameMode);

        // Step 6: empty the warmup island.
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(It->Get());
            if (!PC)
                continue;
            // LastDamager/LastFallInstigator zeroing (+0x7f8/+0x808), 0x142377ac0, 0x142a3baf0 not ported.
            if (APawn* Pawn = PC->GetPawn())
            {
                if (AFortPlayerPawn* FortPawn = Cast<AFortPlayerPawn>(Pawn))
                {
                    // 0x142314fe0 / 0x14234d960(Pawn, 0) (bIsInAnyStorm = 0) / slot +0x1010(1) (inside safe zone), then
                    // OnEnteredAircraft.
                    FortPawn->bIsInAnyStorm = 0;
                    FortPawn->bIsInsideSafeZone = 1;
                    FortPawn->OnEnteredAircraft.Broadcast();
                }
                Pawn->Destroy();
            }
            // Controller slot +0x5d0 then 0x1447ba3f0(PC, NAME_Spectating). Inferred: ChangeState + ClientGotoState.
            PC->bPlayerIsWaiting = false;  // keeps ServerRestartPlayer from respawning while Spectating
            PC->ChangeState(NAME_Spectating);
            PC->ClientGotoState(NAME_Spectating);
        }
        // Step 7: game session StartAircraft / joins closed: no game session here.

        // Step 8.
        if (Playlist->SafeZoneStartUp == ESafeZoneStartUp::StartsWithAirCraft)
            IsSafeZoneAllowed(GameMode, false, false);

        // Step 9: the setter runs the handler, which places players in the aircraft.
        SetGamePhase(GameState, EAthenaGamePhase::Aircraft);
        // 0x141975490 (BuildingGameplayActorSpawnData reset, supply drops) not ported.
        // Step 10: 60 s timer, callback object vtable 0x1459a9ed8 not read.
        LogOnce(TEXT("aircraft-60s"), TEXT("StartAircraftPhase: 0x141975490 and the 60 s timer (vtable 0x1459a9ed8) not ported"));
        return true;
    }

    void WarmupEnd(AFortGameModeAthena* GameMode)
    {
        if (StartAircraftPhase(GameMode, false))
            return;
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        if (!GameState)
            return;
        if (SafeZoneStartUp(GameState) == ESafeZoneStartUp::StartsWithNoAirCraft)
        {
            // 0x14199e430 (the other condition) not read.
            if (!IsSafeZoneAllowed(GameMode, true, false) && GameState->MapInfo)
                SetGamePhase(GameState, EAthenaGamePhase::None);  // argument shown as 0 in the decompile
            for (FConstPlayerControllerIterator It = GameMode->GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(It->Get());
                if (PC && GameMode->PlayerCanRestart(PC))
                    GameMode->RestartPlayer(PC);
            }
            return;
        }
        SetGamePhase(GameState, EAthenaGamePhase::None);
    }

    // ---------------------------------------------------------------------------------------------------------------
    // StartWarmupPhase 0x141972c30 and UpdateWarmupPhase 0x14197a400 (R1 3-4)
    // ---------------------------------------------------------------------------------------------------------------
    bool StartWarmupPhase(AFortGameModeAthena* GameMode)
    {
        FMatchFlowState& S = State();
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        if (!GameState)
            return false;
        UFortPlaylistAthena* Playlist = GetPlaylist(GameState);
        if (Playlist && Playlist->SafeZoneStartUp == ESafeZoneStartUp::StartsWithWarmUp)
            IsSafeZoneAllowed(GameMode, false, false);
        if (S.bWarmupStarted)
            return true;
        S.bWarmupStarted = true;
        // 0x141eacd80 (394 lines, a parent or zone setup call) not read.
        LogOnce(TEXT("prewarmup"), TEXT("StartWarmupPhase: pre-warmup call 0x141eacd80 not ported"));
        if (S.bSkipWarmup)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: Skipping warmup phase"));
            return false;
        }
        S.WarmupStartWorldTime = GameMode->GetWorld()->GetTimeSeconds();
        if (!S.bSkipAircraft)
        {
            for (int32 Index = 0; Index < AircraftCount(GameState); ++Index)
            {
                if (AFortAthenaAircraft* Aircraft = SpawnAircraft(GameMode->GetWorld(), GameState, Index))
                    GameMode->Aircrafts.Add(Aircraft);
            }
        }
        else
        {
            GameState->bGameModeWillSkipAircraft = true;
        }
        SetGamePhase(GameState, EAthenaGamePhase::Warmup);
        // bPauseWarmup from the runtime option byte +0xa4: not available; the URL/command-line option stays.
        if (Playlist && Playlist->WarmupEarlyRequiredPlayerPercent > 0.0f)
            S.EarlyRequiredPlayerPercent = Playlist->WarmupEarlyRequiredPlayerPercent;
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: warmup started: duration %.1f s, early duration %.1f s, early percent %.2f, players %d"),
            GameMode->WarmupCountdownDuration, GameMode->WarmupEarlyCountdownDuration, S.EarlyRequiredPlayerPercent, GameMode->GetNumPlayers());
        return true;
    }

    void UpdateWarmupPhase(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
    {
        FMatchFlowState& S = State();
        if (S.bPauseWarmup)
            return;
        const float NowTime = Now(GameState);
        const float End = GameState->WarmupCountdownEndTime;
        if (End < 0.0f)
        {
            GameState->WarmupCountdownStartTime = NowTime;
            GameState->WarmupCountdownEndTime = NowTime + GameMode->WarmupCountdownDuration;
            GameState->ForceNetUpdate();
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: Warmup phase initiated: StartTime %.2f, EndTime %.2f, NumPlayers %d"),
                GameState->WarmupCountdownStartTime, GameState->WarmupCountdownEndTime, GameMode->GetNumPlayers());
            return;
        }
        if (End < NowTime)
        {
            // Mutator veto (slot +0xa88): no mutators.
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: Warmup countdown is over. Attempting to start aircraft phase."));
            if (GameMode->GetNumPlayers() < 1)
            {
                // 0x1419602e0 (session restart timer) not ported; keep waiting.
                LogOnce(TEXT("warmup-empty"), TEXT("All players disconnected during warmup phase. Restarting session. (restart 0x1419602e0 not ported; warmup stays open)"));
                return;
            }
            S.PlayersAtWarmupEnd = CountReadyPlayers(GameMode->GetWorld());
            WarmupEnd(GameMode);
            return;
        }
        if (!S.bEarlyCountdownStarted && End - NowTime > GameMode->WarmupEarlyCountdownDuration)
        {
            // 0x14199e380 (mutator query, result unused) left out.
            const int32 Ready = CountReadyPlayers(GameMode->GetWorld());
            // MaxPlayerCount: inferred to be the playlist's max players (GetMaxPlaylistPlayers).
            const int32 MaxPlayers = FMath::Max(1, GameState->GetMaxPlaylistPlayers());
            if (S.EarlyRequiredPlayerPercent > 0.0f && Ready >= FMath::CeilToInt(S.EarlyRequiredPlayerPercent * MaxPlayers))
            {
                GameState->WarmupCountdownEndTime = NowTime + GameMode->WarmupEarlyCountdownDuration;
                S.bEarlyCountdownStarted = true;
                GameState->ForceNetUpdate();
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: Player percent reached to start early countdown. EndTime %.2f"), GameState->WarmupCountdownEndTime);
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Bus exit (jump re-creation per D5 3 / R5 4.2, and the forced exit of OnAircraftExitedDropZone 0x141952190)
    // ---------------------------------------------------------------------------------------------------------------
    bool ExitAircraft(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* PC, const FRotator& Rotation, bool bJump)
    {
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        UFortControllerComponent_Aircraft* Component = AircraftComponent(PC);
        if (!GameState || !Component || GameState->GamePhase != EAthenaGamePhase::Aircraft || !Component->NativeRecoveredBool_0xf8)
            return false;
        if (bJump && GameState->bAircraftIsLocked)
        {
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: jump refused for %s: aircraft locked"), *PC->GetName());
            return false;
        }
        FBusPlayerState& Player = State().Players.FindOrAdd(TWeakObjectPtr<AController>(PC));
        AFortAthenaAircraft* Aircraft = Cast<AFortAthenaAircraft>(Component->CurrentAircraft);
        // The component's exit function (writer of +0xf9) is not found yet (R5 4.2); write the end state directly.
        Component->NativeRecoveredBool_0xf8 = false;
        Component->CurrentAircraft = nullptr;
        Player.bLeftAircraft = true;
        Player.bPendingExit = true;
        Player.bJump = bJump;
        Player.ExitRotation = Rotation;
        Player.ExitAircraft = Aircraft;
        if (AFortPlayerStateZone* PlayerState = Cast<AFortPlayerStateZone>(PC->PlayerState))
        {
            PlayerState->bInAircraft = 0;
            PlayerState->ForceNetUpdate();
        }
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: %s leaves the aircraft (%s)"), *PC->GetName(), bJump ? TEXT("jump") : TEXT("forced exit"));
        GameMode->RestartPlayer(PC);
        // inferred: tell the client its component left the aircraft (body of ClientExitAircraft not recovered).
        Component->ClientExitAircraft();
        return true;
    }

    void OnAircraftExitedDropZone(AFortGameModeAthena* GameMode, AFortAthenaAircraft* Aircraft)
    {
        FMatchFlowState& S = State();
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        if (!GameState)
            return;
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: aircraft %s exited the drop zone"), Aircraft ? *Aircraft->GetName() : TEXT("none"));
        TArray<AFortPlayerControllerAthena*> Controllers;
        for (FConstPlayerControllerIterator It = GameMode->GetWorld()->GetPlayerControllerIterator(); It; ++It)
            if (AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(It->Get()))
                Controllers.Add(PC);
        for (AFortPlayerControllerAthena* PC : Controllers)
        {
            // GetAircraftIndexFromTeam is 0 for the default behavior.
            if (GameState->GetAircraft(0) != Aircraft)
                continue;
            UFortControllerComponent_Aircraft* Component = AircraftComponent(PC);
            if (!Component)
                continue;
            const FBusPlayerState* Player = S.Players.Find(TWeakObjectPtr<AController>(PC));
            const bool bLeft = Player && Player->bLeftAircraft;
            if (Component->NativeRecoveredBool_0xf8)
            {
                ExitAircraft(GameMode, PC, PC->GetControlRotation(), false);
            }
            else if (!bLeft || !PC->bReadyToStartMatch)
            {
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: Player [%s] failed to load before aircraft phase ended. Kicking player."), *PC->GetName());
                LogOnce(TEXT("kick"), TEXT("kick through the game session (slot +0x698) not ported; player kept"));
            }
        }
        ++S.AircraftsExitedDropZone;
        if (S.AircraftsExitedDropZone == GameState->Aircrafts.Num())
        {
            const ESafeZoneStartUp StartUp = SafeZoneStartUp(GameState);
            if (StartUp == ESafeZoneStartUp::StartsWithWarmUp || StartUp == ESafeZoneStartUp::StartsWithAirCraft)
                StartSafeZonesPhase(GameMode, false);
            else
                IsSafeZoneAllowed(GameMode, false, true);
        }
    }

    // stand-in: AFortAthenaAircraft's tick (moving the bus and firing the drop-zone delegates) is not ported. The
    // driver moves the server copy along FlightInfo and fires the events at the replicated times.
    void TickAircraft(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
    {
        FMatchFlowState& S = State();
        const float NowTime = Now(GameState);
        TArray<TWeakObjectPtr<AFortAthenaAircraft>> Done;
        for (auto& Pair : S.AircraftRuns)
        {
            AFortAthenaAircraft* Aircraft = Pair.Key.Get();
            FAircraftRunState& Run = Pair.Value;
            if (!Aircraft)
            {
                Done.Add(Pair.Key);
                continue;
            }
            if (Aircraft->FlightStartTime < 0.0f)
                continue;
            const FAircraftFlightInfo& Info = Aircraft->FlightInfo;
            const float Elapsed = FMath::Clamp(NowTime - Aircraft->FlightStartTime, 0.0f, FMath::Max(0.0f, Info.TimeTillFlightEnd));
            Aircraft->FlightElapsedTime = NowTime - Aircraft->FlightStartTime;
            Aircraft->SetActorLocation(FVector(Info.FlightStartLocation) + Info.FlightStartRotation.Vector() * Info.FlightSpeed * Elapsed);
            if (Run.LastTimestampReplication < 0.0f || NowTime - Run.LastTimestampReplication >= NativeAircraftTimestampReplicationInterval)
            {
                Run.LastTimestampReplication = NowTime;
                Aircraft->ReplicatedFlightTimestamp = NowTime;
            }
            if (!Run.bEnteredDropZone && NowTime >= Aircraft->DropStartTime)
            {
                Run.bEnteredDropZone = true;
                Aircraft->NativeInDropZone = 1;
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: aircraft %s entered the drop zone at %.2f"), *Aircraft->GetName(), NowTime);
                struct { AFortAthenaAircraft* FortAthenaAircraft; } Params{ Aircraft };
                Aircraft->NativeAircraftEnteredDropZone.ProcessMulticastDelegate<UObject>(&Params);
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: bAircraftIsLocked=%d after drop-zone entry"), GameState->bAircraftIsLocked ? 1 : 0);
            }
            if (!Run.bExitedDropZone && NowTime >= Aircraft->DropEndTime)
            {
                Run.bExitedDropZone = true;
                Aircraft->NativeInDropZone = 0;  // inferred
                OnAircraftExitedDropZone(GameMode, Aircraft);
            }
            if (!Run.bFlightEnded && NowTime >= Aircraft->FlightEndTime)
            {
                Run.bFlightEnded = true;
                Aircraft->NativeFlightFinished = 1;
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: aircraft %s flight ended at %.2f; destroying"), *Aircraft->GetName(), NowTime);
                struct { AFortAthenaAircraft* FortAthenaAircraft; } Params{ Aircraft };
                Aircraft->NativeAircraftFlightEnded.ProcessMulticastDelegate<UObject>(&Params);
                Done.Add(Pair.Key);
            }
        }
        for (const TWeakObjectPtr<AFortAthenaAircraft>& Key : Done)
            S.AircraftRuns.Remove(Key);
    }

    // stand-in: skydive and parachute physics (PhysCustom of UFortMovementComp_Character) are not ported on the server.
    // While a pawn is skydiving the server takes the client's position and movement mode (both CMC flags set in
    // OnPlayerRestarted). Normal server authority returns once the mirrored mode is no longer custom and the pawn is on
    // the ground.
    void RestoreMovementAuthority(UCharacterMovementComponent* Movement)
    {
        if (!Movement)
            return;
        Movement->bIgnoreClientMovementErrorChecksAndCorrection = false;
        Movement->bServerAcceptClientAuthoritativePosition = false;
    }

    // The Apollo_Background level (cloud sheets, skybox meshes) is decoration. The server loads its static-mesh collision,
    // which makes a skydiver "land" on BP_GiantCloudSheet at about Z 52000. Switch collision off for every actor in it.
    void DisableBackgroundCollision(UWorld* World)
    {
        if (!World)
            return;
        int32 Count = 0;
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            ULevel* Level = It->GetLevel();
            if (Level && Level->GetOutermost() && Level->GetOutermost()->GetName().Contains(TEXT("Apollo_Background")) && It->GetActorEnableCollision())
            {
                It->SetActorEnableCollision(false);
                ++Count;
            }
        }
        if (Count)
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: collision disabled on %d actors of the Apollo_Background level"), Count);
    }

    void TickSkydivers(UWorld* World)
    {
        FMatchFlowState& S = State();
        const float NowTime = World->GetTimeSeconds();
        TArray<TWeakObjectPtr<APawn>> Done;
        for (auto& Pair : S.Skydivers)
        {
            AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Pair.Key.Get());
            if (!Pawn)
            {
                Done.Add(Pair.Key);
                continue;
            }
            UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement();
            FSkydiverState& Diver = Pair.Value;
            const bool bClientOnGround = Movement && Movement->MovementMode != MOVE_Custom && Movement->IsMovingOnGround();
            if (!Diver.bLanded)
            {
                FHitResult Hit;
                FCollisionQueryParams Query(FName(TEXT("MatchFlowLanding")), false, Pawn);
                const FVector From = Pawn->GetActorLocation();
                const bool bNearGround = World->LineTraceSingleByChannel(Hit, From, From - FVector(0.0f, 0.0f, 300.0f), ECC_Visibility, Query);
                if (bClientOnGround || bNearGround)
                {
                    Diver.bLanded = true;
                    Diver.LandedTime = NowTime;
                    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteProbeLanding")))
                    {
                        // Diagnostic: everything solid below the landing point, to see which collision the server really has.
                        TArray<FHitResult> Hits;
                        FCollisionQueryParams MultiQuery(FName(TEXT("MatchFlowLandingProbe")), false, Pawn);
                        World->LineTraceMultiByChannel(Hits, FVector(From.X, From.Y, 30000.0f), FVector(From.X, From.Y, -20000.0f), ECC_Visibility, MultiQuery);
                        for (int32 HitIndex = 0; HitIndex < Hits.Num() && HitIndex < 10; ++HitIndex)
                            UE_LOG(LogTemp, Display, TEXT("MatchFlow: landing probe hit %d: z=%.1f %s [%s]"), HitIndex, Hits[HitIndex].ImpactPoint.Z,
                                Hits[HitIndex].GetActor() ? *Hits[HitIndex].GetActor()->GetPathName() : TEXT("none"),
                                Hits[HitIndex].GetComponent() ? *Hits[HitIndex].GetComponent()->GetClass()->GetName() : TEXT("none"));
                        if (Hits.Num() == 0)
                            UE_LOG(LogTemp, Display, TEXT("MatchFlow: landing probe: no collision under %s"), *From.ToString());
                        int32 LevelsLoaded = 0, LevelsVisible = 0;
                        for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
                        {
                            if (StreamingLevel && StreamingLevel->IsLevelLoaded()) ++LevelsLoaded;
                            if (StreamingLevel && StreamingLevel->IsLevelVisible()) ++LevelsVisible;
                        }
                        UE_LOG(LogTemp, Display, TEXT("MatchFlow: landing probe: %d streaming levels, %d loaded, %d visible"), World->GetStreamingLevels().Num(), LevelsLoaded, LevelsVisible);
                        int32 Shown = 0;
                        for (TActorIterator<ALandscapeProxy> LandscapeIt(World); LandscapeIt && Shown < 6; ++LandscapeIt)
                        {
                            const FBox LandscapeBox = LandscapeIt->GetComponentsBoundingBox(true);
                            const FVector2D Near(FMath::Clamp(From.X, LandscapeBox.Min.X, LandscapeBox.Max.X), FMath::Clamp(From.Y, LandscapeBox.Min.Y, LandscapeBox.Max.Y));
                            if (FVector2D::Distance(Near, FVector2D(From.X, From.Y)) > 25000.0f)
                                continue;
                            int32 Registered = 0, Total = 0;
                            for (UActorComponent* Comp : LandscapeIt->GetComponents())
                            {
                                if (Comp && Comp->GetClass()->GetName().Contains(TEXT("HeightfieldCollision")))
                                {
                                    ++Total;
                                    if (Comp->IsRegistered()) ++Registered;
                                }
                            }
                            ++Shown;
                            UE_LOG(LogTemp, Display, TEXT("MatchFlow: landing probe landscape %s level %s bounds %s collision comps %d registered %d"),
                                *LandscapeIt->GetName(), LandscapeIt->GetLevel() ? *LandscapeIt->GetLevel()->GetOutermost()->GetName() : TEXT("?"),
                                *LandscapeBox.ToString(), Total, Registered);
                        }
                    }
                    Pawn->bIsSkydiving = false;
                    Pawn->bIsParachuteOpen = false;
                    Pawn->bIsSkydivingFromBus = 0;
                    Pawn->ForceNetUpdate();
                    UE_LOG(LogTemp, Display, TEXT("MatchFlow: %s reached the ground at %s (mode %s, client on ground %d, trace hit %s / %s at %s)"), *Pawn->GetName(), *From.ToString(),
                        Movement ? *Movement->GetMovementName() : TEXT("none"), bClientOnGround ? 1 : 0,
                        Hit.GetActor() ? *Hit.GetActor()->GetPathName() : TEXT("none"), Hit.GetComponent() ? *Hit.GetComponent()->GetClass()->GetName() : TEXT("none"),
                        *Hit.ImpactPoint.ToString());
                }
            }
            if (Diver.bLanded && (bClientOnGround || NowTime - Diver.LandedTime > 10.0f))
            {
                // The 10 s limit only covers a client that never reports a ground mode.
                if (Movement && Movement->MovementMode == MOVE_Custom)
                    Movement->SetMovementMode(MOVE_Falling);
                RestoreMovementAuthority(Movement);
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: %s landed, server movement authority restored (mode %s)"), *Pawn->GetName(),
                    Movement ? *Movement->GetMovementName() : TEXT("none"));
                Done.Add(Pair.Key);
            }
        }
        for (const TWeakObjectPtr<APawn>& Key : Done)
            S.Skydivers.Remove(Key);
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Storm: activation 0x141968920, advance 0x14196ede0, per-frame update 0x141978230 (R2 4-6)
    // ---------------------------------------------------------------------------------------------------------------
    void ApplyIndicatorRadius(AFortSafeZoneIndicator* Indicator, float Radius, const FVector& Center)  // 0x141a5b1f0
    {
        Indicator->Radius = Radius;
        if (Indicator->SafeZoneMesh)
        {
            // inferred: world transform of the component at +0x3a0
            Indicator->SafeZoneMesh->SetWorldTransform(FTransform(FRotator::ZeroRotator, Center,
                FVector(Radius * Indicator->SafeZoneWorldScale, Radius * Indicator->SafeZoneWorldScale, 1000.0f)));
        }
    }

    void SetNextNext(AFortGameModeAthena* GameMode, AFortSafeZoneIndicator* Indicator, AFortAthenaMapInfo* MapInfo, int32 Phase)
    {
        // 0x141964450 (only its writes were read): NextNextCenter / NextNextRadius / thickness from phase + 1.
        const int32 Next = Phase + 1;
        const bool bValid = GameMode->SafeZoneLocations.IsValidIndex(Next);
        Indicator->NextNextCenter = bValid ? GameMode->SafeZoneLocations[Next] : FVector(Indicator->NextCenter);
        Indicator->NextNextRadius = bValid ? SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.Radius, Next) : Indicator->NextRadius;
        Indicator->NextNextMegaStormGridCellThickness = FMath::RoundToInt(SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.MegaStormGridCellThickness, Next));
    }

    void ActivateSafeZone(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
    {
        AFortAthenaMapInfo* MapInfo = GameState->MapInfo;
        GameMode->bSafeZoneActive = true;
        GameMode->SafeZonePhase = 0;
        GameState->SafeZonePhase = 0;
        // 0x1419635f0, 0x1419634f0, 0x141975490 and mutators (slot +0x738) not ported.
        AFortSafeZoneIndicator* Indicator = GameMode->SafeZoneIndicator;
        if (!Indicator)
        {
            if (!GameMode->SafeZoneIndicatorClass)
            {
                LogOnce(TEXT("no-indicator-class"), TEXT("no SafeZoneIndicatorClass; storm cannot be shown"));
                return;
            }
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Indicator = GameMode->GetWorld()->SpawnActor<AFortSafeZoneIndicator>(GameMode->SafeZoneIndicatorClass, FTransform::Identity, Params);
            if (!Indicator)
            {
                UE_LOG(LogTemp, Warning, TEXT("MatchFlow: SafeZoneIndicator spawn failed"));
                return;
            }
            Indicator->SetReplicates(true);  // stand-in: force replication on the spawned Blueprint
            Indicator->bAlwaysRelevant = true;
            GameMode->SafeZoneIndicator = Indicator;
        }
        const FVector Center0 = GameMode->SafeZoneLocations.IsValidIndex(0) ? GameMode->SafeZoneLocations[0] : FVector::ZeroVector;
        Indicator->LastCenter = Center0;
        Indicator->NextCenter = Center0;
        GameState->SafeZoneIndicator = Indicator;
        CallUFunction(GameState, TEXT("OnRep_SafeZoneIndicator"));
        const float NowTime = Now(GameState);
        SetShrinkTimes(Indicator, MapInfo, 0, NowTime);
        const float Radius0 = FMath::Max(MapInfo->SafeZoneStartingRadius.GetValueAtLevel(0.0f), SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.Radius, 0));
        Indicator->LastRadius = Radius0;
        Indicator->NextRadius = Radius0;
        Indicator->NextMegaStormGridCellThickness = FMath::RoundToInt(SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.MegaStormGridCellThickness, 0));
        if (GameMode->MegaStormManager)
            LogOnce(TEXT("megastorm-delay"), TEXT("MegaStormDelayTimeBeforeDestruction copy from MegaStormManager (+0x22c) not ported"));
        SetNextNext(GameMode, Indicator, MapInfo, 0);
        ApplyIndicatorRadius(Indicator, Radius0, Center0);
        Indicator->ForceNetUpdate();
        GameState->ForceNetUpdate();
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: safe zone active: indicator %s replicates=%d centre %s radius %.0f shrink %.2f-%.2f"),
            *Indicator->GetName(), Indicator->GetIsReplicated() ? 1 : 0, *Center0.ToString(), Radius0,
            Indicator->SafeZoneStartShrinkTime, Indicator->SafeZoneFinishShrinkTime);
    }

    void AdvanceSafeZone(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState, int32 ForcedIndex)
    {
        AFortAthenaMapInfo* MapInfo = GameState->MapInfo;
        AFortSafeZoneIndicator* Indicator = GameMode->SafeZoneIndicator;
        if (!MapInfo || !Indicator)
            return;
        const int32 Count = SafeZonePhaseCount(MapInfo);
        GameMode->SafeZonePhase = ForcedIndex < 0 ? GameMode->SafeZonePhase + 1 : ForcedIndex;
        const int32 P = GameMode->SafeZonePhase;
        // 0x1419af220 is an empty stub in the client (compiled-out server code); inferred to set the replicated copy.
        GameState->SafeZonePhase = static_cast<uint8>(P);
        const TArray<FVector>& Locations = GameMode->SafeZoneLocations;
        if (ForcedIndex < 0)
        {
            Indicator->LastCenter = Indicator->NextCenter;
            Indicator->NextCenter = Locations.IsValidIndex(P) ? Locations[P] : FVector(Indicator->NextCenter);
            Indicator->LastRadius = Indicator->Radius;  // 0x141a48240
            Indicator->NextRadius = SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.Radius, P);
        }
        else
        {
            Indicator->LastCenter = Locations.IsValidIndex(P) ? Locations[P] : FVector::ZeroVector;
            Indicator->NextCenter = Locations.IsValidIndex(P + 1) ? Locations[P + 1] : FVector(Indicator->LastCenter);
            Indicator->LastRadius = SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.Radius, P);
            Indicator->NextRadius = P + 1 < Count ? SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.Radius, P + 1) : Indicator->LastRadius;
        }
        Indicator->NextMegaStormGridCellThickness = FMath::RoundToInt(SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.MegaStormGridCellThickness, P));
        SetNextNext(GameMode, Indicator, MapInfo, P);
        SetShrinkTimes(Indicator, MapInfo, P, Now(GameState));
        // Step 7 mega storm: thickness 0 in the default rows; skipped.
        Indicator->ForceNetUpdate();
        // Step 9: per-pawn effect level refresh is done in TickStormPawns. Step 10 (normal advance only).
        if (ForcedIndex < 0)
            ApplyIndicatorRadius(Indicator, SafeZoneValue(MapInfo, MapInfo->SafeZoneDefinition.Radius, P), FVector(Indicator->NextCenter));
        // Step 11 SkyTubeDisablePhase, steps 13-14 (analytics, respawn data) not ported.
        GameMode->OnSafeZoneUpdated.Broadcast();
        GameState->ForceNetUpdate();
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: safe zone phase %d: %s r%.0f -> %s r%.0f, shrink %.2f-%.2f"), P,
            *FVector(Indicator->LastCenter).ToString(), Indicator->LastRadius, *FVector(Indicator->NextCenter).ToString(), Indicator->NextRadius,
            Indicator->SafeZoneStartShrinkTime, Indicator->SafeZoneFinishShrinkTime);
    }

    void UpdateSafeZones(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
    {
        const float NowTime = Now(GameState);
        if (!GameMode->bSafeZoneActive && GameState->SafeZonesStartTime >= 0.0f && NowTime > GameState->SafeZonesStartTime)
            ActivateSafeZone(GameMode, GameState);
        AFortSafeZoneIndicator* Indicator = GameMode->SafeZoneIndicator;
        if (GameState->SafeZonePauseTime >= 0.0f || !Indicator || !GameState->MapInfo)
            return;
        const EAthenaGamePhase Phase = GameState->GamePhase;
        if (Phase == EAthenaGamePhase::Warmup || Phase == EAthenaGamePhase::Aircraft)
        {
            if (!Indicator->bPausedForPreview && GameMode->SafeZonePhase == 0 && NowTime >= Indicator->SafeZoneFinishShrinkTime && !Indicator->bSafezoneEventDriven)
                AdvanceSafeZone(GameMode, GameState, -1);
            Indicator->bPausedForPreview = true;
            return;
        }
        if (Indicator->bPausedForPreview)
            Indicator->bPausedForPreview = false;
        if (NowTime > Indicator->SafeZoneFinishShrinkTime && !Indicator->bSafezoneEventDriven &&
            GameMode->SafeZonePhase < SafeZonePhaseCount(GameState->MapInfo) - 1)
        {
            const UFortPlaylistAthena* Playlist = GetPlaylist(GameState);
            if (Playlist && Playlist->LastSafeZoneIndex >= 0 && Playlist->LastSafeZoneIndex == GameMode->SafeZonePhase)
            {
                if (!GameState->bPlaylistStoppedSafeZonePhases)  // 0x1419b0f50, countdown part not ported
                {
                    GameState->bPlaylistStoppedSafeZonePhases = true;
                    UE_LOG(LogTemp, Display, TEXT("MatchFlow: playlist LastSafeZoneIndex %d reached; storm stopped"), Playlist->LastSafeZoneIndex);
                }
                return;
            }
            AdvanceSafeZone(GameMode, GameState, -1);
        }
    }

    // stand-in for IsInsideSafeZone 0x14194e9e0 (not read): the indicator has no ported tick, so interpolate the
    // circle between Last and Next over the shrink window.
    bool IsInsideSafeZone(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState, const FVector& Location)
    {
        const AFortSafeZoneIndicator* Indicator = GameMode->SafeZoneIndicator;
        if (!GameMode->bSafeZoneActive || !Indicator)
            return true;
        const float NowTime = Now(GameState);
        const float Duration = Indicator->SafeZoneFinishShrinkTime - Indicator->SafeZoneStartShrinkTime;
        float Alpha = 0.0f;
        if (NowTime >= Indicator->SafeZoneFinishShrinkTime)
            Alpha = 1.0f;
        else if (NowTime > Indicator->SafeZoneStartShrinkTime && Duration > 0.0f)
            Alpha = (NowTime - Indicator->SafeZoneStartShrinkTime) / Duration;
        const FVector Center = FMath::Lerp(FVector(Indicator->LastCenter), FVector(Indicator->NextCenter), Alpha);
        const float Radius = FMath::Lerp(Indicator->LastRadius, Indicator->NextRadius, Alpha);
        return FVector::DistSquared2D(Center, Location) <= FMath::Square(Radius);
    }

    // Storm inside check 0x141961aa0 and per-pawn storm effect 0x1419622e0 (R6 3, 3.1), ordinary-match branch (bFlag false).
    void TickStormPawns(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
    {
        FMatchFlowState& S = State();
        const float WorldTime = GameMode->GetWorld()->GetTimeSeconds();
        static const FGameplayTag InsideTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.InsideSafeZone")), false);
        for (FConstPlayerControllerIterator It = GameMode->GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            AFortPlayerPawn* Pawn = It->IsValid() ? Cast<AFortPlayerPawn>(It->Get()->GetPawn()) : nullptr;
            if (!Pawn)
                continue;
            const bool bWarmup = GameState->GamePhase == EAthenaGamePhase::Warmup;
            const bool bInside = bWarmup ? true : IsInsideSafeZone(GameMode, GameState, Pawn->GetActorLocation());
            const int32 Phase = GameMode->SafeZonePhase;
            // Camping timer (StormCamping* rows are 0 by default, the timer never starts): not ported.
            // Step 2: the DogSweater tag is not checked (no such item here), so bIsInAnyStorm = !bInside.
            if (!bWarmup && Pawn->bIsInAnyStorm != (bInside ? 0 : 1))
            {
                Pawn->bIsInAnyStorm = bInside ? 0 : 1;  // 0x14233c4e0 screen effects skipped (cosmetic)
            }
            // Step 3: slot +0x1010 = 0x14234d990.
            if (Pawn->bIsInsideSafeZone != (bInside ? 1 : 0))
            {
                Pawn->bIsInsideSafeZone = bInside ? 1 : 0;
                UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
                if (ASC && InsideTag.IsValid())
                    ASC->SetLooseGameplayTagCount(InsideTag, bInside ? 1 : 0);
                Pawn->OnSafeZoneOccupancyChangedEvent.Broadcast(bInside);
                Pawn->ForceNetUpdate();
                UE_LOG(LogTemp, Display, TEXT("MatchFlow: %s is now %s the safe zone (phase %d)"), *Pawn->GetName(), bInside ? TEXT("inside") : TEXT("outside"), Phase);
            }
            if (bWarmup)
                continue;
            // Steps 4-6: effect level = phase; StormCampingDamage set when the effect was applied (OnPlayerRestarted).
            FPawnStormState* Storm = S.PawnStorm.Find(TWeakObjectPtr<APawn>(Pawn));
            UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
            if (ASC && Storm && Storm->Handle.IsValid())
            {
                ASC->SetActiveGameplayEffectLevel(Storm->Handle, Phase);
            }
            // stand-in: UFortDamageFormulaExecutionCalculation (the effect's execution) has no ported body, so the effect
            // cannot deal damage. Apply Default.SafeZone.Damage (fraction of max health per 1 s period) directly.
            if (!bInside && GameMode->bSafeZoneActive)
            {
                FPawnStormState& StandIn = S.PawnStorm.FindOrAdd(TWeakObjectPtr<APawn>(Pawn));
                if (StandIn.LastStandInDamageTime < 0.0f || WorldTime - StandIn.LastStandInDamageTime >= 1.0f)
                {
                    StandIn.LastStandInDamageTime = WorldTime;
                    if (!Pawn->HealthSet || !S.GameData.IsValid())
                    {
                        LogOnce(TEXT("storm-nohealth"), TEXT("storm damage not applied: pawn has no HealthSet (or AthenaGameData missing); G8 attribute setup needed"));
                        continue;
                    }
                    FScalableFloat DamageRow;
                    DamageRow.SetScalingValue(1.0f, FName(TEXT("Default.SafeZone.Damage")), S.GameData.Get());
                    const float Damage = DamageRow.GetValueAtLevel(static_cast<float>(Phase)) * Pawn->GetMaxHealth();
                    // Through the health module so a fatal tick runs the death sequence (storm damage ignores shields, D6 3).
                    FGameplayTagContainer StormTags;
                    StormTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("DeathCause.Storm")), false));
                    if (FortAthenaHealth::IsEnabled())
                        FortAthenaHealth::ApplyDamage(Pawn, Damage, StormTags, nullptr, nullptr, true);
                    else
                        Pawn->SetHealth(FMath::Max(0.0f, Pawn->GetHealth() - Damage));
                    UE_LOG(LogTemp, Display, TEXT("MatchFlow: storm damage %.1f to %s, health %.1f (stand-in)"), Damage, *Pawn->GetName(), Pawn->GetHealth());
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Per-frame driver: stand-in for the unported game mode Tick 0x141975090 (R1 2), run from a world timer.
    // ---------------------------------------------------------------------------------------------------------------
    void LogStatus(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
    {
        // R1 10 / R2 9 probe values, every 5 s.
        FString Players;
        for (FConstPlayerControllerIterator It = GameMode->GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            AFortPlayerController* PC = Cast<AFortPlayerController>(It->Get());
            if (!PC)
                continue;
            const UFortControllerComponent_Aircraft* Component = AircraftComponent(PC);
            Players += FString::Printf(TEXT(" [%s ready=%d state=%s pawn=%s inAircraft=%d]"), *PC->GetName(), PC->bReadyToStartMatch ? 1 : 0,
                *PC->GetStateName().ToString(), PC->GetPawn() ? *PC->GetPawn()->GetName() : TEXT("none"),
                Component && Component->NativeRecoveredBool_0xf8 ? 1 : 0);
        }
        const AFortSafeZoneIndicator* Indicator = GameMode->SafeZoneIndicator;
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: status t=%.1f phase=%s warmup=%.1f..%.1f ready=%d aircraft gm=%d gs=%d locked=%d exited=%d szActive=%d szStart=%.1f szPhase=%d r=%.0f->%.0f%s"),
            Now(GameState), PhaseName(GameState->GamePhase), GameState->WarmupCountdownStartTime, GameState->WarmupCountdownEndTime,
            CountReadyPlayers(GameMode->GetWorld()), GameMode->Aircrafts.Num(), GameState->Aircrafts.Num(), GameState->bAircraftIsLocked ? 1 : 0,
            State().AircraftsExitedDropZone, GameMode->bSafeZoneActive ? 1 : 0, GameState->SafeZonesStartTime, GameMode->SafeZonePhase,
            Indicator ? Indicator->LastRadius : 0.0f, Indicator ? Indicator->NextRadius : 0.0f, *Players);
    }

    void DriverTick()
    {
        FMatchFlowState& S = State();
        AFortGameModeAthena* GameMode = GetGameMode();
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        if (!GameMode || !GameState || !S.bSetupDone)
            return;
        UWorld* World = GameMode->GetWorld();
        const float WorldTime = World->GetTimeSeconds();
        const float Delta = S.LastDriverTime < 0.0f ? 0.0f : WorldTime - S.LastDriverTime;
        S.LastDriverTime = WorldTime;
        FortAthenaInventory::Tick(World);
        FortAthenaEndGame::Tick(GameMode, Delta);

        // HandleMatchHasStarted -> StartWarmupPhase. inferred: deferred until RequiredPlayers have joined (the original
        // waits for the match to start; with no player the countdown would end in the "all players disconnected" branch).
        if (!S.bWarmupStarted)
        {
            if (GameMode->GetNumPlayers() < S.RequiredPlayers)
                return;
            DisableBackgroundCollision(GameMode->GetWorld());  // streamed levels may have finished after the driver armed
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: %d of %d required players joined; starting warmup"), GameMode->GetNumPlayers(), S.RequiredPlayers);
            if (!StartWarmupPhase(GameMode))
                WarmupEnd(GameMode);
        }

        const EAthenaGamePhase Phase = GameState->GamePhase;
        const ESafeZoneStartUp StartUp = SafeZoneStartUp(GameState);
        // Step 1: storm inside check 0x141961aa0.
        TickStormPawns(GameMode, GameState);
        // Step 3: safe-zone update.
        if ((Phase == EAthenaGamePhase::Warmup && StartUp == ESafeZoneStartUp::StartsWithWarmUp) ||
            (Phase == EAthenaGamePhase::Aircraft && (StartUp == ESafeZoneStartUp::StartsWithWarmUp || StartUp == ESafeZoneStartUp::StartsWithAirCraft)) ||
            Phase == EAthenaGamePhase::SafeZones)
        {
            UpdateSafeZones(GameMode, GameState);
            // SpawnActorDataList countdown (0x141967e60) not ported.
        }
        // Step 4.
        if (Phase == EAthenaGamePhase::Warmup)
            UpdateWarmupPhase(GameMode, GameState);
        // Step 5: 0x141978a80 (supply drops) not ported. Step 6: match time.
        if (Phase == EAthenaGamePhase::Warmup || Phase == EAthenaGamePhase::Aircraft || Phase == EAthenaGamePhase::SafeZones)
            S.MatchTime += Delta;
        // Steps 7-8 (supply drop list, storm cap 0x1419797b0, 0x1419783c0, mutator ticks) not ported.
        TickAircraft(GameMode, GameState);
        TickSkydivers(World);

        if (S.LastStatusTime < 0.0f || WorldTime - S.LastStatusTime >= 5.0f)
        {
            S.LastStatusTime = WorldTime;
            LogStatus(GameMode, GameState);
        }
    }

    bool HasOption(AFortGameModeAthena* GameMode, const TCHAR* Key)
    {
        // Every URL option is also accepted as a command-line switch of the same name (R1 12).
        return UGameplayStatics::HasOption(GameMode->OptionsString, Key) || FParse::Param(FCommandLine::Get(), Key);
    }
}


namespace FortAthenaMatchFlow
{
    using namespace FortAthenaMatchFlowPrivate;

    bool IsActive()
    {
        return State().bActive;
    }

    void Start(AFortGameModeAthena* GameMode)
    {
        FMatchFlowState& S = State();
        if (!OriginalClientMode() || !GameMode || !GameMode->GetWorld() || S.bActive || S.GameMode.IsValid())
            return;
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        if (!GameState)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: no AFortGameStateAthena; not started"));
            return;
        }
        S.GameMode = GameMode;

        // InitGame options (D2 2, R1 12). Defaults: 80 required players; config DefaultGame.ini
        // [/Script/FortniteGame.FortGameModeAthena] WarmupCountdownDuration=90, WarmupEarlyCountdownDuration=15,
        // WarmupEarlyRequiredPlayerPercent=0.95 (data). The config keys may not reach our class; fill when unset.
        S.RequiredPlayers = UGameplayStatics::GetIntOption(GameMode->OptionsString, TEXT("RequiredPlayers"), 80);
        FParse::Value(FCommandLine::Get(), TEXT("RequiredPlayers="), S.RequiredPlayers);
        S.RequiredPlayers = FMath::Max(1, S.RequiredPlayers);
        GameMode->WarmupRequiredPlayerCount = S.RequiredPlayers;
        S.bSkipWarmup = HasOption(GameMode, TEXT("SkipWarmup"));
        S.bPauseWarmup = HasOption(GameMode, TEXT("PauseWarmup"));
        S.bSkipAircraft = HasOption(GameMode, TEXT("SkipAircraft"));
        S.bPauseAircraft = HasOption(GameMode, TEXT("PauseAircraft"));
        if (GameMode->WarmupCountdownDuration <= 0.0f)
            GameMode->WarmupCountdownDuration = 90.0f;
        if (GameMode->WarmupEarlyCountdownDuration <= 0.0f)
            GameMode->WarmupEarlyCountdownDuration = 15.0f;
        S.EarlyRequiredPlayerPercent = GameMode->DefaultWarmupEarlyRequiredPlayerPercent > 0.0f ? GameMode->DefaultWarmupEarlyRequiredPlayerPercent : 0.95f;
        // stand-in: -FortniteWarmupSeconds=<N> shortens the warmup countdown for test runs.
        float WarmupSeconds = 0.0f;
        if (FParse::Value(FCommandLine::Get(), TEXT("FortniteWarmupSeconds="), WarmupSeconds) && WarmupSeconds > 0.0f)
            GameMode->WarmupCountdownDuration = WarmupSeconds;
        // stand-in test options for the storm.
        FParse::Value(FCommandLine::Get(), TEXT("FortniteSafeZoneTimeScale="), S.SafeZoneTimeScale);
        S.SafeZoneTimeScale = FMath::Max(0.001f, S.SafeZoneTimeScale);
        S.bStormCentered = FParse::Param(FCommandLine::Get(), TEXT("FortniteStormCentered"));
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: options RequiredPlayers=%d SkipWarmup=%d PauseWarmup=%d SkipAircraft=%d PauseAircraft=%d warmup=%.1fs storm time scale=%.3f centred=%d"),
            S.RequiredPlayers, S.bSkipWarmup ? 1 : 0, S.bPauseWarmup ? 1 : 0, S.bSkipAircraft ? 1 : 0, S.bPauseAircraft ? 1 : 0,
            GameMode->WarmupCountdownDuration, S.SafeZoneTimeScale, S.bStormCentered ? 1 : 0);

        S.bSetupDone = SetupPlaylistAndMap(GameMode, GameState);
        S.bActive = S.bSetupDone;  // a failed setup leaves the G6 stand-ins in charge
        if (!S.bSetupDone)
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: setup failed; match flow driver not armed"));
            return;
        }
        GameMode->GetWorldTimerManager().SetTimer(S.DriverHandle, FTimerDelegate::CreateStatic(&DriverTick), DriverInterval, true);
        DisableBackgroundCollision(GameMode->GetWorld());
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: driver armed (%.2f s); waiting for %d player(s)"), DriverInterval, S.RequiredPlayers);
    }

    void OnPlayerRestarted(AFortGameModeAthena* GameMode, AController* Controller)
    {
        FMatchFlowState& S = State();
        if (!S.bActive || !GameMode || !Controller)
            return;
        AFortGameStateAthena* GameState = GetGameState(GameMode);
        AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Controller->GetPawn());
        if (!GameState || !Pawn)
            return;

        // FinishRestartPlayer applies GE_OutsideSafeZone once per pawn (D2 10, R6 3). The ability system of our server
        // pawns is not set up yet, so this usually logs once and stops.
        UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
        if (ASC && GameMode->GE_OutsideSafeZone)
        {
            FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GameMode->GE_OutsideSafeZone, static_cast<float>(GameMode->SafeZonePhase), ASC->MakeEffectContext());
            if (Spec.IsValid())
            {
                static const FGameplayTag CampingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("SetByCaller.StormCampingDamage")), false);
                if (CampingTag.IsValid())
                    Spec.Data->SetSetByCallerMagnitude(CampingTag, 1.0f);  // max(1.0, camping multiplier) with the default rows
                S.PawnStorm.FindOrAdd(TWeakObjectPtr<APawn>(Pawn)).Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }
        else
        {
            LogOnce(TEXT("ge-not-applied"), FString::Printf(TEXT("GE_OutsideSafeZone not applied to %s: ability system component %s, effect class %s"),
                *Pawn->GetName(), ASC ? TEXT("present") : TEXT("missing"), GameMode->GE_OutsideSafeZone ? TEXT("present") : TEXT("missing")));
        }

        FBusPlayerState* Player = S.Players.Find(TWeakObjectPtr<AController>(Controller));
        if (!Player || !Player->bPendingExit)
            return;
        Player->bPendingExit = false;
        AFortAthenaAircraft* Aircraft = Player->ExitAircraft.Get();
        const FVector BusLocation = Aircraft ? Aircraft->GetActorLocation() : FVector(0.0f, 0.0f, 80000.0f);
        // stand-in: TeleportToSkyDive 0x141a267c0 and BeginSkydiving are real client code but not ported to our pawn.
        // Place the pawn below the bus, face the jump rotation and mark it skydiving from the bus.
        const FVector DropLocation = BusLocation - FVector(0.0f, 0.0f, 500.0f);
        Pawn->SetActorLocationAndRotation(DropLocation, FRotator(0.0f, Player->ExitRotation.Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
        Pawn->bIsSkydiving = true;
        Pawn->bIsSkydivingFromBus = 1;
        Pawn->bIsParachuteOpen = false;
        if (UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement())
        {
            Movement->SetMovementMode(MOVE_Custom, static_cast<uint8>(EFortCustomMovement::Skydiving));
            // stand-in: no server PhysCustom; skip error checks and take the client's skydive/parachute position and mode
            // until landing (ServerMoveHandleClientError only reaches the accept branch when no error is detected).
            Movement->bIgnoreClientMovementErrorChecksAndCorrection = true;
            Movement->bServerAcceptClientAuthoritativePosition = true;
        }
        if (Player->bJump)
        {
            // FinishRestartPlayer reset the control rotation to the start spot; restore the jump rotation.
            if (APlayerController* PC = Cast<APlayerController>(Controller))
            {
                PC->SetControlRotation(Player->ExitRotation);
                PC->ClientSetRotation(Player->ExitRotation, true);
            }
        }
        Pawn->ForceNetUpdate();
        S.Skydivers.FindOrAdd(TWeakObjectPtr<APawn>(Pawn));
        UE_LOG(LogTemp, Display, TEXT("MatchFlow: %s skydiving from the bus at %s (%s)"), *Pawn->GetName(), *DropLocation.ToString(),
            Player->bJump ? TEXT("jump") : TEXT("forced exit"));
    }
}

void FortAthenaMatchFlow_ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* Component, const FRotator& ClientRotation)
{
    // Re-creation of the compiled-out UFortControllerComponent_Aircraft::ServerAttemptAircraftJump (D5 3, R1 6, R5 4.2):
    // phase Aircraft, bus unlocked, player in the aircraft, then the same RestartPlayer path as the forced exit.
    using namespace FortAthenaMatchFlowPrivate;
    if (!FortAthenaMatchFlow::IsActive() || !Component)
        return;
    AFortGameModeAthena* GameMode = GetGameMode();
    AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(Component->GetOwner());
    UE_LOG(LogTemp, Display, TEXT("MatchFlow: ServerAttemptAircraftJump from %s rotation %s"), PC ? *PC->GetName() : TEXT("none"), *ClientRotation.ToString());
    if (GameMode && PC)
        ExitAircraft(GameMode, PC, ClientRotation, true);
}

void FortAthenaMatchFlow_OnReadyToStartMatch(AFortPlayerController* Controller)
{
    // OnReadyToStartMatch 0x14239ccf0 steps 1-3 (R4 1). Steps 4-7 (StatManager, quick bar objects, persistent stats)
    // are not ported.
    using namespace FortAthenaMatchFlowPrivate;
    if (!OriginalClientMode() || !Controller || Controller->bReadyToStartMatch)
        return;
    UE_LOG(LogTemp, Display, TEXT("MatchFlow: OnReadyToStartMatch: Marking player %s ready to start"), *Controller->GetName());
    Controller->bReadyToStartMatch = 1;
    LogOnce(TEXT("ready-tail"), TEXT("OnReadyToStartMatch steps 4-7 (StatManager, quick bar objects, stats) not ported"));
}

// stand-in: the two Server RPC bodies above are bound at run time, because the generated _Implementation bodies are the
// empty stub (jump) or "body not recovered" (ready) and generated code is not hand-edited. Remove when they are spliced
// through behavior/*.inc and the generator. _Validate of both is the shared "return true" stub (0x1406801d0).
namespace
{
    void ExecServerAttemptAircraftJumpOverride(UObject* Context, FFrame& Stack, RESULT_DECL)
    {
        P_GET_STRUCT(FRotator, ClientRotation);
        P_FINISH;
        P_NATIVE_BEGIN;
        FortAthenaMatchFlow_ServerAttemptAircraftJump(Cast<UFortControllerComponent_Aircraft>(Context), ClientRotation);
        P_NATIVE_END;
    }

    void ExecServerReadyToStartMatchOverride(UObject* Context, FFrame& Stack, RESULT_DECL)
    {
        P_FINISH;
        P_NATIVE_BEGIN;
        FortAthenaMatchFlow_OnReadyToStartMatch(Cast<AFortPlayerController>(Context));
        P_NATIVE_END;
    }

    void Rebind(UClass* Class, const TCHAR* Name, FNativeFuncPtr Func)
    {
        if (UFunction* Function = Class->FindFunctionByName(Name))
        {
            Function->SetNativeFunc(Func);
            UE_LOG(LogTemp, Display, TEXT("MatchFlow: %s::%s rebound to the re-created body"), *Class->GetName(), Name);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MatchFlow: %s::%s UFunction not found; RPC stays empty"), *Class->GetName(), Name);
        }
    }
}

void FortAthenaMatchFlow::InstallRpcOverrides()
{
    static bool bInstalled = false;
    if (bInstalled || !FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")))
        return;
    bInstalled = true;
    Rebind(UFortControllerComponent_Aircraft::StaticClass(), TEXT("ServerAttemptAircraftJump"), &ExecServerAttemptAircraftJumpOverride);
    Rebind(AFortPlayerController::StaticClass(), TEXT("ServerReadyToStartMatch"), &ExecServerReadyToStartMatchOverride);
}
