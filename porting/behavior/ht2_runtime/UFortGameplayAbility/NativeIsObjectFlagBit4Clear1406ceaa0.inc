// Helper 0x1406ceaa0, host UFortGameplayAbility (static). 5 instructions: MOV EAX,[RCX+0x8]; SHR EAX,0x4; NOT AL; AND AL,0x1; RET.
// UObject+0x8 has no reflected member (lookup_offset.py UObject 0x8), so the uint32 at +0x8 is read raw.
// Result is the inverse of bit 0x10 of that dword (UE4 EObjectFlags value 0x10 is RF_ClassDefaultObject; name kept neutral).
// Callers: UFortAbilityTask_WaitTargetSelection::OnTargetDataReadyCallback (0x141632740) and UFortGameplayAbility::ClearCameraMode (0x1415eefe0) pass RCX = this object.
// CONFIDENCE: HIGH
const uint32 ObjectFlagWord = *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(TargetObject) + 0x8);
return (ObjectFlagWord & 0x10u) == 0;
