// Helper 0x1440b2120, host UFortMinigameObjectiveComponent (static).
// Evidence: MOV RAX,[RCX+0xa0]; TEST RAX,RAX; JZ (return 0); MOVZX EAX,byte ptr [RAX+0xf0]; RET.
// Context is the calling component; the pointer at +0xa0 is read and the byte at +0xf0 of that object is returned.
// Callers (UAthenaResurrectionComponent, UBuildingTurretComponent) compare the result with 0x2.
// No reflected member covers +0xa0/+0xf0 on the caller side, so access is raw.
// CONFIDENCE: MEDIUM
const uint8* ObjectiveRaw = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(Context) + 0xa0);
if (!ObjectiveRaw)
{
    return 0;
}
return *reinterpret_cast<const uint8*>(ObjectiveRaw + 0xf0);
