// CONFIDENCE: HIGH. Complete original body; shared producer lifecycle remains separate.
// AFortPlayerControllerGameplay 0x1423d9510; original assumes a valid World.
// 140f95ae0 = &GetWorld()->GetTimerManager(), 144621fb0 = FindTimer.
// Status +1 != 2 means != ETimerStatus::Paused (includes Pending/Executing).
return GetWorld()->GetTimerManager().IsTimerActive(NativeInputBlockTimer2568);
