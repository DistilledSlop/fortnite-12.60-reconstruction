// CONFIDENCE: HIGH
//14211f770. NativeControllerTick225b.h + Engine/World.h + AFortGameStateAthena.h.
if (bNativeControllerVirtual1d10Started) return;
UWorld* World = GetWorld();
if (!World) return;
AFortGameStateAthena* AthenaState = Cast<AFortGameStateAthena>(World->GetGameState());
if (!AthenaState || static_cast<uint8>(AthenaState->GamePhase) < 3) return;
bNativeControllerVirtual1d10Started = true;
FControllerState225b& NativeState = NativeControllerState141ca4780();
NativeStartControllerTickable1420fe440(NativeState, this);
