// Native helper 0x141ca1b30 (static helper on UFortGameplayAbility; one caller: ClearCameraMode body 0x1415eefe0).
// Asm: MOV EAX,[RDX]; CMP [RCX+0x2710],EAX; JNZ exit; MOV qword [RCX+0x2708],0; MOV dword [RCX+0x2710],-1; RET.
// RCX = target object (caller passes the object reached through this+0x380 weak pointer and +0x2b0); RDX = const int32* key.
// Offsets 0x2708 (pointer-sized slot) and 0x2710 (int32 key) have no reflected members; raw access with the asm widths.
// CONFIDENCE: HIGH
uint8* const ObjectBytes = reinterpret_cast<uint8*>(TargetObject);
if (*reinterpret_cast<const int32*>(ObjectBytes + 0x2710) == *Key)
{
	*reinterpret_cast<UClass**>(ObjectBytes + 0x2708) = nullptr;
	*reinterpret_cast<int32*>(ObjectBytes + 0x2710) = -1;
}
