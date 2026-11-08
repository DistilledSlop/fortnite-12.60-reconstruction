// Behaviour snippet: UFortMinigameStatics::RemoveAllInventoryItems (body only; spliced into the generated static function).
// Original address: 0x142483e40 (thunk 0x1429d8ce0)
// Offset evidence: ASOLFortPlayer+0x278 = NativeRecoveredPointer_0x278 (uint64, unreflected pointer in the class's own region). Helper 0x142414c60 = NativeHelper142414c60 (declared static). The controller IsA(AFortPlayerControllerAthena) gate tail-jumps to the empty helper 0x1406801c0, so it contributes no effect and is folded out; only the helper call remains.
// CONFIDENCE: MEDIUM
static_assert(sizeof(ASOLFortPlayer) > 0, "ASOLFortPlayer must be complete to read the Player pointer member");
if (Player == nullptr)
{
	return;
}
UObject* Context = reinterpret_cast<UObject*>(Player->NativeRecoveredPointer_0x278);
(void)UFortMinigameStatics::NativeHelper142414c60(Context);
