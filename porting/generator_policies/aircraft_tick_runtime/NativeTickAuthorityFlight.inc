// CONFIDENCE: high
// Complete authority subpath of original Tick 0x1418d7fb0; caller owns authority/lifecycle selection.
if (NativeFlightFinished || FlightStartTime < 0.0f)
        return;
    const float Now = GameState->GetServerWorldTimeSeconds();
    FlightElapsedTime = Now - FlightStartTime;
    if (Now - ReplicatedFlightTimestamp > NativeAircraftTimestampReplicationInterval)
        ReplicatedFlightTimestamp = Now;
    // Preserve multiply order; no elapsed-time clamp or movement replication.
    SetActorLocation((FlightInfo.FlightStartRotation.Vector() * FlightInfo.FlightSpeed) * FlightElapsedTime
                     + FVector(FlightInfo.FlightStartLocation), false, nullptr, ETeleportType::None);

    if (GetLocalRole() == ROLE_Authority)
    {
        const bool WasInDropZone = NativeInDropZone != 0;
        NativeInDropZone = Now > DropStartTime && Now < DropEndTime;
        struct { AFortAthenaAircraft* FortAthenaAircraft; } Params { this };
        if (!WasInDropZone)
        {
            if (NativeInDropZone)
            {
                NativeAircraftEnteredDropZone.ProcessMulticastDelegate<UObject>(&Params);
                // Re-read after script callbacks, which may mutate this actor.
                if (bIsOutOfPhaseAircraft)
                    GameState->SetAircraftLocked(false);
            }
        }
        else if (!NativeInDropZone)
        {
            NativeAircraftExitedDropZone.ProcessMulticastDelegate<UObject>(&Params);
        }
        if (!NativeFlightFinished && Now > FlightEndTime)
        {
            NativeFlightFinished = 1;
            if (NativeInDropZone)
            {
                NativeInDropZone = 0;
                Params.FortAthenaAircraft = this;
                NativeAircraftExitedDropZone.ProcessMulticastDelegate<UObject>(&Params);
            }
            Params.FortAthenaAircraft = this;
            NativeAircraftFlightEnded.ProcessMulticastDelegate<UObject>(&Params);
            if (bIsOutOfPhaseAircraft)
                Destroy(false, true);
        }
    }
