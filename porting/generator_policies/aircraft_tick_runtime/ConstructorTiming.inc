// Exact timing/flight constructor statements, not the complete constructor.
// AFortAthenaAircraft 0x1418b4050, stores 0x1418b40ad..40dc / 4261..42c1.
FlightInfo.FlightStartLocation = FVector::ZeroVector;
FlightInfo.FlightStartRotation = FRotator::ZeroRotator;
FlightInfo.FlightSpeed = 0.0f;
FlightInfo.TimeTillFlightEnd = -1.0f;
FlightInfo.TimeTillDropStart = -1.0f;
FlightInfo.TimeTillDropEnd = -1.0f;
bIsOutOfPhaseAircraft = false;
FlightStartTime = -1.0f;
FlightEndTime = -1.0f;
DropStartTime = -1.0f;
DropEndTime = -1.0f;
FlightElapsedTime = 0.0f;
NativeInDropZone = 0;
NativeFlightFinished = 0;
MiniMapIconScale = 0.5f;
MiniMapTeamIndicatorIconScale = 0.5f;
NumSpawnSlots = 10;
SpawnOffsetRadius = 200.0f;
AircraftIndex = 0;
// ReplicatedFlightTimestamp and ClientFlightTimerDrift have no explicit ctor stores.
// Retain UObject zero initialization; do not initialize either to server time.
