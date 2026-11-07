// Behaviour snippet: AFortCreativeMoveTool::IncrementOrWrapHitTraceRuleOnClient (body only; spliced into the generated member function).
// Original address: 0x1426772c0
// Offset evidence: INC byte at +0x114b (NativeRecoveredUint8_0x114b; EHitTraceType TraceType is a uint8 at +0x1149 and does not cover it); wrap when the new value is above 2 (JBE skips the reset); OnMoveToolLineOfSightBlockingChanged delegate at exact +0xcd8 is broadcast with the new value (the byte passed in AL).
// CONFIDENCE: MEDIUM
uint8 NewRule = static_cast<uint8>(NativeRecoveredUint8_0x114b + 1);
NativeRecoveredUint8_0x114b = NewRule;
if (NewRule > 2)
{
	NativeRecoveredUint8_0x114b = 0;
	NewRule = 0;
}
OnMoveToolLineOfSightBlockingChanged.Broadcast(static_cast<EHitTraceRule>(NewRule));
