// Behaviour snippet: UFortMinigameStatics::SetInfiniteAmmo (body only; spliced into the generated static function).
// Original address: 0x142487760 (thunk 0x1429d8f50)
// Offset evidence: ASOLFortPlayer+0x278 = NativeRecoveredPointer_0x278 (uint64). Helper 0x142414c60 = NativeHelper142414c60 (declared static) returns the controller. Controller IsA(AFortPlayerControllerAthena) gate, then dword AND ~0x4 / OR (infinite << 2) at controller+0x1cb4 = AFortPlayerController::bInfiniteAmmo (declared bit, mask 0x4), so the RMW pair is one bit assignment.
// CONFIDENCE: MEDIUM
static_assert(sizeof(ASOLFortPlayer) > 0, "ASOLFortPlayer must be complete to read the Player pointer member");
static_assert(sizeof(AFortPlayerControllerAthena) > 0, "AFortPlayerControllerAthena must be complete for the bInfiniteAmmo bit");
if (Player == nullptr)
{
	return;
}
UObject* Context = reinterpret_cast<UObject*>(Player->NativeRecoveredPointer_0x278);
if (Context == nullptr)
{
	return;
}
uint8* Controller = UFortMinigameStatics::NativeHelper142414c60(Context);
if (Controller == nullptr)
{
	return;
}
UObject* ControllerObject = reinterpret_cast<UObject*>(Controller);
if (!ControllerObject->IsA(AFortPlayerControllerAthena::StaticClass()))
{
	return;
}
reinterpret_cast<AFortPlayerControllerAthena*>(Controller)->bInfiniteAmmo = infinite;
