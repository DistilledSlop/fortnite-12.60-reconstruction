// Original address: 0x141a78120 (UFortGameStateComponent_EventLevel helper)
// Offset evidence: SceneCameras TArray (reflected at +0x1b0, Num at +0x1b8). Elements are 8-byte TWeakObjectPtr; asm calls
// IsValid (0x142ee7880) then Get (0x142ee5dd0). Match compares the full 64-bit value at object +0x7b8 with the 64-bit key argument
// (asm compares both 32-bit halves). Returns the first match, else null. Callers read [result+0x7b8] themselves.
// CONFIDENCE: HIGH
for (const TWeakObjectPtr<AFortEventLevelCamera>& CameraCandidate : ComponentObj->SceneCameras)
{
    if (!CameraCandidate.IsValid())
    {
        continue;
    }
    AFortEventLevelCamera* CameraObject = CameraCandidate.Get();
    if (*reinterpret_cast<const uint64*>(reinterpret_cast<const uint8*>(CameraObject) + 0x7b8) == CameraKeyValue)
    {
        return CameraObject;
    }
}
return nullptr;
