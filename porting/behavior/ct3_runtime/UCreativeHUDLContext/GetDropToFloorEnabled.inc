// Original body 0x144ccab90 (thunk 0x1451e1b60).
// Offset evidence: [this+0x140] = CurrentMoveTool (reflected Object). Null tool returns true (MOV AL,1 path).
// Pending-kill tool (GUObjectArray item flags bit 29) returns true. Out-of-range index path is not modelled.
// Final read is the raw byte [tool+0x1119] returned whole (MOVZX), not the bAllowGravityOnPlace bit member.
// CONFIDENCE: MEDIUM
AFortCreativeMoveTool* Tool = CurrentMoveTool;
if (Tool == nullptr)
{
	return true;
}
static_assert(sizeof(AFortCreativeMoveTool) > 0, "AFortCreativeMoveTool must be complete");
if (Tool->IsPendingKill())
{
	return true;
}
const uint8* Raw = reinterpret_cast<const uint8*>(Tool);
return Raw[0x1119] != 0;
