// Original body address: 0x14267d680 (thunk 0x1428c3ac0)
// Offset evidence: CreativeOption null exits with bare RET at 0x14267d6e4. CALL 0x141d85c50 = UFortCreativeOption::GetValueAtIndex(int32). MOV dword [+0x1154]: 0x114c is WantedHitTraceRule (FCreativeOptionVariableBase: FGameplayTag 8 bytes, int32 Value at +8), so +0x1154 is WantedHitTraceRule.Value. MOV byte [+0x114b]: gap after TraceType (0x1149), NativeRecoveredByte_0x114b. Delegate at 0xcd8 = OnMoveToolLineOfSightBlockingChanged (one EHitTraceRule param, byte in AL); RCX=this+0xcd8 at CALL 0x1406a9dc0.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortCreativeOption) > 0, "UFortCreativeOption must be complete for GetValueAtIndex");
if (!CreativeOption)
{
    return;
}
const int32 RuleValue = CreativeOption->GetValueAtIndex(static_cast<int32>(Index));
WantedHitTraceRule.Value = RuleValue;
uint8 RuleByte = 2;
if (RuleValue == 0)
{
    RuleByte = 0;
}
else if (RuleValue == 1)
{
    RuleByte = 1;
}
NativeRecoveredByte_0x114b = RuleByte;
OnMoveToolLineOfSightBlockingChanged.Broadcast(static_cast<EHitTraceRule>(RuleByte));
