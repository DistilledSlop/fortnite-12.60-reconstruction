// Behaviour snippet: UFortControllerComponent_Ghost::IsInGhostMode (body only; spliced into the generated member function).
// Original address: 0x141cd92f0
// Offset evidence: object+0xa0 holds a UObject pointer (below the class's lowest own offset 0xb8, raw access). Owner->ClassPrivate is tested against AFortPlayerControllerGameplay::StaticClass() (UStruct::IsChildOf fast path). Result is byte [owner+0x2448] != 0 (GhostModeRepData region, raw). Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change); the failing path returns false.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
UObject* Controller = *reinterpret_cast<UObject* const*>(Raw + 0xa0);
if (Controller == nullptr || !Controller->IsA(AFortPlayerControllerGameplay::StaticClass()))
{
	return false;
}
const uint8* ControllerRaw = reinterpret_cast<const uint8*>(Controller);
return ControllerRaw[0x2448] != 0;
