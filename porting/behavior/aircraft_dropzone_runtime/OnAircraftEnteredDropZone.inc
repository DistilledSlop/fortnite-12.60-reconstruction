// Original address: 0x1428f0440; native body 0x1419520f0.
// CONFIDENCE: HIGH
// Native body 0x1419520f0. The reflected aircraft parameter is unused.
// A failed cast intentionally remains unchecked, as in the original body.
AFortGameStateAthena* AthenaGameState = Cast<AFortGameStateAthena>(GameState);
bool bAllAircraftInDropZone = true;
for (int32 Index = 0; Index < AthenaGameState->Aircrafts.Num(); ++Index)
{
    AFortAthenaAircraft* Aircraft = AthenaGameState->GetAircraft(Index);
    if (Aircraft && Aircraft->NativeInDropZone == 0)
    {
        bAllAircraftInDropZone = false;
    }
}
if (bAllAircraftInDropZone)
{
    AthenaGameState->SetAircraftLocked(false);
}
