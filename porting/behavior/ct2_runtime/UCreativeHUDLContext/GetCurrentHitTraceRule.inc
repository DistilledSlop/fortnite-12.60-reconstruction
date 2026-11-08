// Behaviour snippet: UCreativeHUDLContext::GetCurrentHitTraceRule (body only).
// Original body address: 0x144cc9e90 (thunk 0x1451e1b00). Null or pending-kill CurrentMoveTool returns enum value 2.
// Offset evidence: [this+0x140] = CurrentMoveTool (reflected AFortCreativeMoveTool*). [tool+0x114b] byte is an unreflected gap in AFortCreativeMoveTool (member_field NativeRecoveredUint8_0x114b).
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortCreativeMoveTool) > 0, "AFortCreativeMoveTool must be complete for the byte read");
AFortCreativeMoveTool* MoveToolPtr = CurrentMoveTool;
if (!MoveToolPtr || MoveToolPtr->IsPendingKill())
{
	return static_cast<EHitTraceRule>(2);
}
return static_cast<EHitTraceRule>(MoveToolPtr->NativeRecoveredUint8_0x114b);
