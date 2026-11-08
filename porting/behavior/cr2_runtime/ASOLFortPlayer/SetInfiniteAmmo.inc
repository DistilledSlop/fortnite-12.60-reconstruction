// Original body: 0x1424876f0 (thunk 0x142b908b0). Offset evidence: [RCX+0x278] is the existing member NativeRecoveredUint64_0x278 (object offset 0x278). The helper result must be an AFortPlayerControllerAthena (IsA check); then byte 0x1cb4 bit 0x4 is written from the infinite argument, which is AFortPlayerController::bInfiniteAmmo (declared bitfield, bool_bits.json mask 0x4).
// CONFIDENCE: MEDIUM
UObject* Context = reinterpret_cast<UObject*>(NativeRecoveredUint64_0x278);
if (Context == nullptr)
{
	return;
}
uint8* HelperResult = UFortMinigameStatics::NativeHelper142414c60(Context);
AFortPlayerControllerAthena* AthenaController = Cast<AFortPlayerControllerAthena>(reinterpret_cast<UObject*>(HelperResult));
if (AthenaController == nullptr)
{
	return;
}
AthenaController->bInfiniteAmmo = infinite;
