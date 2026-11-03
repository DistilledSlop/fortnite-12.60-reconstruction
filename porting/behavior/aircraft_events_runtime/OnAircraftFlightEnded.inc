// Original address: 0x1428f0540; native body 0x141952500.
// CONFIDENCE: HIGH
// The original attempts a GameState cast and retains the StaticClass call only.
if (GameState)
{
    AFortGameStateAthena::StaticClass();
}
if (FortAthenaAircraft)
{
    FortAthenaAircraft->NativeAircraftEnteredDropZone.Remove(this, FName(TEXT("OnAircraftEnteredDropZone")));
    FortAthenaAircraft->NativeAircraftExitedDropZone.Remove(this, FName(TEXT("OnAircraftExitedDropZone")));
    FortAthenaAircraft->NativeAircraftFlightEnded.Remove(this, FName(TEXT("OnAircraftFlightEnded")));
    FortAthenaAircraft->Destroy(false, true);
}
