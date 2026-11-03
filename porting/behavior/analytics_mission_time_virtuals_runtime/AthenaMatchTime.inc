// CONFIDENCE: HIGH
return static_cast<uint8>(GamePhase) >= static_cast<uint8>(EAthenaGamePhase::Aircraft)
        ? GetServerWorldTimeSeconds() - AircraftStartTime : 0.0f;
