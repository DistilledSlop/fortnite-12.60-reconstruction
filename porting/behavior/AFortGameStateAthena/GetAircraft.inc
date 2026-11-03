// CONFIDENCE: HIGH
// Behaviour snippet: AFortGameStateAthena::GetAircraft (body only).
// Original address: 0x14291e730; native body 0x141990d90.
// Decompile source: porting/generated/decomp/AFortGameStateAthena/GetAircraft.c
// Evidence: porting/behavior/aircraft_lookup_evidence.md
const bool bValidIndex = Aircrafts.IsValidIndex(AircraftIndex);
if (GamePhase == EAthenaGamePhase::Setup || GamePhase == EAthenaGamePhase::Warmup)
{
	return bValidIndex ? Aircrafts[AircraftIndex] : nullptr;
}

if (!bValidIndex)
{
	UE_LOG(LogFort, Verbose, TEXT("[AFortGameStateAthena::GetAircraft()] Aircraft Index is invalid. This might be ok. Index is [%d], num of Aircraft is [%d]"), AircraftIndex, Aircrafts.Num());
}
else if (Aircrafts[AircraftIndex] != nullptr)
{
	return Aircrafts[AircraftIndex];
}

UWorld* World = GetWorld();
const bool bPlayingReplay = World != nullptr && World->IsPlayingReplay();
if (!bPlayingReplay)
{
	UE_LOG(LogFort, Warning, TEXT("AFortGameStateAthena::GetAircraft Aircrafts[%d] is not valid."), AircraftIndex);
}
return nullptr;
