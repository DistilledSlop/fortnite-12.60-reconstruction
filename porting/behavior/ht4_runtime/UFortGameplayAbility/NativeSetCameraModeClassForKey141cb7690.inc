// Native helper 0x141cb7690 (static helper on UFortGameplayAbility; one caller: SetCameraMode body 0x14160c620).
// Asm: TEST RDX,RDX; JZ exit; CALL UFortCameraMode::StaticClass (0x142856e60); JZ exit when null; then the inline
// class-tree IsChildOf test with RDX itself as the class (ancestor table at RDX+0x30, depth at RDX+0x38) against the returned class.
// On success: [RDI+0x2708] = RDX (class pointer) and [RDI+0x2710] = *R8 (int32 key). Offsets have no reflected members; raw access.
// CONFIDENCE: MEDIUM
if (CameraModeClass == nullptr)
{
	return;
}
UClass* const CameraModeBase = UFortCameraMode::StaticClass();
if (CameraModeBase == nullptr || !CameraModeClass->IsChildOf(CameraModeBase))
{
	return;
}
uint8* const ObjectBytes = reinterpret_cast<uint8*>(TargetObject);
*reinterpret_cast<UClass**>(ObjectBytes + 0x2708) = CameraModeClass;
*reinterpret_cast<int32*>(ObjectBytes + 0x2710) = *Key;
