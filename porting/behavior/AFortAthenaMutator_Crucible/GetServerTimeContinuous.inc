// CONFIDENCE: high; captured native branches and reflected fields.
UWorld* World = GetWorld();
if (!World) return LatestServerTime;
const float WorldTime = World->GetTimeSeconds();
return GetLocalRole() == ROLE_Authority ? WorldTime : (WorldTime - NativeRecoveredFloat_0x52c) + LatestServerTime;
