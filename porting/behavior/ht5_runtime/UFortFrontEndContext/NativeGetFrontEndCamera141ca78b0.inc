// Helper body for UFortFrontEndContext static helper at 0x141ca78b0 (returns the cached camera or null).
// Asm: null-check RCX; RBX = qword [RCX+0x2b0], null-check; RAX = AFortPlayerCameraBase::StaticClass() + 0x30 (CALL 0x142a35cb0);
// RCX = qword [RBX+0x10]; EDX = MOVSXD dword [RAX+0x8]; fail if EDX > dword [RCX+0x38]; fail if qword [[RCX+0x30]+RDX*8] != RAX;
// otherwise return qword [RBX+0x2718]. Every instruction is accounted for.
// Offset evidence: lookup_offset UFortFrontEndContext 0x2b0 and 0x2718 report no reflected member, so raw reads with the asm widths are used.
// Callers: UFortFrontEndContext::GetFrontEndCamera 0x144ccc6f0, SetFrontEndCamera 0x144ce6fd0, UFortGameStateComponent_EventLevel::HandleOnCameraCut 0x141a7fcb0.
// CONFIDENCE: HIGH
if (FrontEndCtx == nullptr)
{
	return nullptr;
}
uint8* SubsystemState = *reinterpret_cast<uint8* const*>(reinterpret_cast<uint8*>(FrontEndCtx) + 0x2b0);
if (SubsystemState == nullptr)
{
	return nullptr;
}
uint8* CameraClassEntry = reinterpret_cast<uint8*>(AFortPlayerCameraBase::StaticClass()) + 0x30;
uint8* EntryTable = *reinterpret_cast<uint8* const*>(SubsystemState + 0x10);
int32 EntryIndex = *reinterpret_cast<const int32*>(CameraClassEntry + 0x8);
if (EntryIndex > *reinterpret_cast<const int32*>(EntryTable + 0x38))
{
	return nullptr;
}
uint8** EntryItems = *reinterpret_cast<uint8***>(EntryTable + 0x30);
if (EntryItems[EntryIndex] != CameraClassEntry)
{
	return nullptr;
}
return *reinterpret_cast<AActor* const*>(SubsystemState + 0x2718);
