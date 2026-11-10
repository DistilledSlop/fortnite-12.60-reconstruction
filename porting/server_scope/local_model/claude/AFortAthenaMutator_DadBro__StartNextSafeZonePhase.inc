// CONFIDENCE: MEDIUM for the phase advance, indicator, table and pawn-apply logic; LOW for the engine helper internals
// (0x141964450, 0x141936680, 0x141781400, 0x1410856e0) identified by role only (see evidence notes).
// Original address: 0x1427fede0 (exec thunk). Captured decompile: 1304 lines. The six inlined copies of the same
// stale-table check are folded into EnsureSafeZoneTables; the unreachable else of the MatchedPhase branch is kept.
if (Role != ROLE_Authority || CachedGameMode == nullptr)
{
    return;
}

AFortGameModeAthena* GM = CachedGameMode;
AGameStateBase* GS = GM->GameState;                                   // +0x278
int32 MatchedPhase = INDEX_NONE;                                      // aiStack_128[0]
if (GS == nullptr)
{
    return;
}
if (!GS->IsA(AFortGameStateAthena::StaticClass()) || GS->SafeZoneTuning == nullptr || GM->SafeZoneIndicator == nullptr)
{
    return;
}

UFortSafeZoneTuning* Tuning = GS->SafeZoneTuning;                     // GameState +0x1c98
UFortSafeZoneIndicator* Indicator = GM->SafeZoneIndicator;            // GameMode +0xfe8

// Rebuild the 11 per-phase float tables when the global table version changed (*(Tuning+0x6c8) != _DAT_148115c60).
auto EnsureSafeZoneTables = [&]()
{
    if (Tuning->TableVersion == GSafeZoneTableVersion)
    {
        return;
    }
    Tuning->TableVersion = GSafeZoneTableVersion;
    Tuning->NumPhases = Tuning->PhaseCountScalable.GetValueAtLevel(0.0f);      // FUN_14070fdf0(+0x548, 0)
    for (int32 k = 0; k < 11; ++k)
    {
        Tuning->PhaseTables[k].Reset();                                         // FUN_14072d2e0
    }
    for (float Level = 0.0f; Level < Tuning->NumPhases; Level += 1.0f)
    {
        for (int32 k = 0; k < 11; ++k)
        {
            Tuning->PhaseTables[k].Add(Tuning->PhaseScalables[k].GetValueAtLevel(Level));   // FScalableFloat + FUN_140690b30
        }
    }
};

// PhaseTables[k][Index], or 0 when the index is out of range (the decompile's bVar5 test).
auto TableAt = [&](int32 k, int32 Index) -> float
{
    const TArray<float>& Table = Tuning->PhaseTables[k];
    return (Index < 0 || Index >= Table.Num()) ? 0.0f : Table[Index];
};

// Phase advance.
GM->PrepareSafeZoneTransition();                                       // FUN_1419635f0
GM->SafeZonePhase += 1;                                                // GameMode +0xfe0
GS->SetSafeZonePhaseByte(static_cast<uint8>(GM->SafeZonePhase));       // FUN_1419af220 (authority only, writes on change)
GM->NotifySafeZonePhaseAdvanced();                                     // FUN_1419634f0
GM->UpdateSafeZoneSpawnables();                                        // FUN_141975490

// Indicator: previous and new zone.
Indicator->PrevCenter = Indicator->CurrCenter;                         // +0x22c <- +0x238
Indicator->PrevRadius = Indicator->CurrRadius;                         // +0x234 <- +0x240
const FVector& PhaseLocation = GM->SafeZoneLocations[GM->SafeZonePhase];
Indicator->CurrCenter = FVector2D(PhaseLocation.X, PhaseLocation.Y);   // +0x238 <- first 8 bytes
Indicator->CurrRadius = PhaseLocation.Z;                               // +0x240 <- third float
Indicator->NumBuildingGridCells = Indicator->GetBuildingGridCellsThick();   // FUN_141a48240 (+0x39c) -> +0x220

EnsureSafeZoneTables();
Indicator->TableValue0 = TableAt(0, GM->SafeZonePhase);                // +0x224

// Grid cell thickness: floor(round(2f + 0.5) / 2) with f = PhaseTables[7][phase]. Written to +0x268.
EnsureSafeZoneTables();
const float GridSource = TableAt(7, GM->SafeZonePhase);
Indicator->GridCellThickness = FMath::RoundToInt(GridSource + GridSource + 0.5f) >> 1;
GM->RefreshSafeZoneGridCells();                                        // FUN_141964450

// Timed radius branch: taken when PhaseTables[5][phase] is missing or non-negative.
EnsureSafeZoneTables();
const bool bIndexValid5 = GM->SafeZonePhase >= 0 && GM->SafeZonePhase < Tuning->PhaseTables[5].Num();
const bool bValue5NonNegative = bIndexValid5 && (Tuning->PhaseTables[5][GM->SafeZonePhase] >= 0.0f);
if (!bIndexValid5 || bValue5NonNegative)
{
    Indicator->bPhaseUsesTimedRadius = false;                          // +0x258 = false
    EnsureSafeZoneTables();
    Indicator->TimedPhaseBase = GS->GetSafeZoneRadiusBias() + TableAt(5, GM->SafeZonePhase);   // vslot 0x630 + table 5 -> +0x250
    EnsureSafeZoneTables();
    Indicator->TimedPhaseEnd = TableAt(6, GM->SafeZonePhase) + Indicator->TimedPhaseBase;      // table 6 + +0x250 -> +0x254
}
else
{
    Indicator->bPhaseUsesTimedRadius = true;                           // +0x258 = true
}

// MegaStorm: build building grid layers for the zone.
const int32 GridLayers = Indicator->GridCellThickness;                 // +0x268
if (GridLayers < 1)
{
    UE_LOG(LogFortGameModeAthena, Warning, TEXT("FortGameModeAthena: No MegaStorm on SafeZone[%d].  GridCellThickness is less than 1.0."), GM->SafeZonePhase);
}
else if (!FortAthenaGameModeCVars::bMegaStormEnabled)
{
    UE_LOG(LogFortGameModeAthena, Warning, TEXT("FortGameModeAthena: No MegaStorm on SafeZone[%d].  FortAthenaGameModeCVars::bMegaStormEnabled is disabled."), GM->SafeZonePhase);
}
else if (GM->MegaStormManager == nullptr)
{
    UE_LOG(LogFortGameModeAthena, Warning, TEXT("FortGameModeAthena: No MegaStorm on SafeZone[%d].  MegaStormManager is invalid"), GM->SafeZonePhase);
}
else
{
    Indicator->PrepareMegaStormGrid();                                 // FUN_141a48210
    int32 NumBuildingGridCellsThick = Indicator->GetBuildingGridCellsThick();   // FUN_141a48240
    UE_LOG(LogFortGameModeAthena, Log, TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."), GM->SafeZonePhase, NumBuildingGridCellsThick);
    for (int32 Layer = GridLayers; Layer >= 0; --Layer)                // do { ... } while (layer >= 0), entered with layer >= 1
    {
        GM->MegaStormManager->BuildGridLayer(Indicator->GridHandle, NumBuildingGridCellsThick, Layer);   // FUN_1419fba60
    }
}

Indicator->UpdateZoneVisuals();                                        // vslot 0x5e8 of the indicator

// Per-actor notification: iterate the world actors of the iterator class and call each component (vslot 0x10 of +0x4c8) with vslot 0x5f0.
UWorld* World = GM->GetWorld();                                        // vslot 0x150
for (AActor* Actor : WorldActorIterator(World, /*class*/ 5, GetIteratorTime()))   // FUN_142a82010 seeds the class, FUN_14192cca0 builds the iterator
{
    if (IActorComponentProvider* Provider = Actor->GetComponentProvider())   // vslot 0x10 of object at +0x4c8
    {
        Provider->OnSafeZoneValue(Actor->SafeZoneValue);                 // vslot 0x5f0 with actor +0x1224
    }
}

// Game state: push the zone into the GameState only when a matched phase exists (never set on the main path; see note).
if (MatchedPhase != INDEX_NONE)
{
    MatchedPhase = GM->SafeZonePhase;
    GS->SetSafeZoneCenter(Indicator->CurrCenter, Indicator->CurrRadius, TableAt(0, MatchedPhase));   // FUN_1419ac150
}
else
{
    // FUN_1419ac150 path not taken; GameState is used as the target below.
}

// Sky tube disable: when the disable phase equals the current phase (and the disable value is positive), disable them.
const int32 SkyTubeDisablePhase = FMath::TruncToInt(GM->SkyTubeDisablePhase.GetValueAtLevel(0.0f));   // FUN_1406f73b0 (+0xfc0)
if (SkyTubeDisablePhase > 0 && SkyTubeDisablePhase == GM->SafeZonePhase)
{
    GS->DisableSkyTubesIfReady();                                        // FUN_1419b0f00
}

// Phase-start multicast: broadcast only when bound (+0xaa8 > 0).
if (GM->OnSafeZonePhaseStarted.IsBound())
{
    GM->OnSafeZonePhaseStarted.Broadcast();                              // FUN_1406a9dc0
}

// New and previous spheres for the GameMode.
FSafeZoneSphere PreviousSphere = MakeSafeZoneSphere(Indicator->PrevCenter, Indicator->PrevRadius, GM->SafeZonePhase - 1);   // FUN_141761060 (+0x22c, +0x220)
FSafeZoneSphere NextSphere     = MakeSafeZoneSphere(Indicator->CurrCenter, Indicator->CurrRadius, GM->SafeZonePhase);       // FUN_141761060 (+0x238, +0x224)
GM->SetSafeZoneSpheres(NextSphere, PreviousSphere);                      // FUN_141781400

// Alive pawns: apply the zone to each eligible pawn (decompile loop over +0xdd0 with count +0xdd8).
for (AController* Controller : GM->AlivePlayers)
{
    APawn* Pawn = (Controller != nullptr) ? Controller->GetPawn() : nullptr;   // controller +0x220
    if (Pawn == nullptr || !IsSafeZoneCandidateClass(Pawn))              // FUN_14192cc60 (FortPlayerStateAthena class check)
    {
        continue;
    }
    if (!Pawn->bSafeZoneEligibleA || !Pawn->bSafeZoneEligibleB)          // bytes +0x12ec and +0xd30
    {
        continue;
    }
    if (IsPawnSafeZoneBlocked(Pawn))                                     // FUN_141a4b2b0 must be false
    {
        continue;
    }
    if (!GM->CanPawnTrackSafeZone(Pawn))                                 // vslot 0x9e0
    {
        continue;
    }
    int32 PawnPhase = INDEX_NONE;
    if (!GM->TestPawnAgainstSafeZone(Pawn, NextSphere, PawnPhase))       // FUN_141936680 (out phase)
    {
        continue;
    }
    Pawn->ApplySafeZoneSphere(PreviousSphere, NextSphere, PawnPhase);    // FUN_141a5b110
    Pawn->NotifySafeZoneApplied();                                       // FUN_1419ebd40
}
