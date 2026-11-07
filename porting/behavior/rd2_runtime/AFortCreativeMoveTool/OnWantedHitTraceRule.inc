// Behaviour snippet: AFortCreativeMoveTool::OnWantedHitTraceRule (body only; spliced into the generated member function).
// Original address: 0x14267d680 (thunk 0x1428c3ac0). Asm: porting/generated/native-cap-direct/0x14267d680.asm
// Offset evidence: WantedHitTraceRule (FCreativeOptionVariableBase, +0x114c) Value int32 at +0x8 = object +0x1154 (stored from GetValueAtIndex result).
// Offset evidence: +0x114b is an unreflected byte between TraceType (+0x1149, uint8 enum) and WantedHitTraceRule (+0x114c); recovered as NativeRecoveredByte_0x114b.
// Offset evidence: OnMoveToolLineOfSightBlockingChanged = InlineMulticastDelegate at +0xcd8, one EHitTraceRule (uint8) parameter, packed at [RSP+0x38].
// Callee: UFortCreativeOption::GetValueAtIndex(int32) at 0x141d85c50 (declared in UFortCreativeOption.h), receiver CreativeOption.
// CONFIDENCE: HIGH
if (CreativeOption == nullptr)
{
    return;
}
const int32 Result = CreativeOption->GetValueAtIndex(static_cast<int32>(Index));
WantedHitTraceRule.Value = Result;
uint8 Rule = 2;
if (Result == 0)
{
    Rule = 0;
}
else if (Result == 1)
{
    Rule = 1;
}
NativeRecoveredByte_0x114b = Rule;
OnMoveToolLineOfSightBlockingChanged.Broadcast(static_cast<EHitTraceRule>(Rule));
