// Original body 0x142614870 (thunk 0x142b29d50). Offset evidence: bool flags at +0x61a (NativeRecoveredBool_0x61a, existing), +0x61d, +0x61e, +0x61f; int32 at +0x628 and +0x62c; all unreflected gaps.
// CONFIDENCE: HIGH
if (NativeRecoveredBool_0x61d)
{
    NativeRecoveredBool_0x61d = false;
    NativeRecoveredInt32_0x628 = -1;
    NativeRecoveredInt32_0x62c = 0;
    NativeRecoveredBool_0x61e = bTraceComplex;
    NativeRecoveredBool_0x61f = false;
    return;
}
if (!NativeRecoveredBool_0x61a)
{
    return;
}
NativeRecoveredBool_0x61d = true;
NativeRecoveredBool_0x61e = bTraceComplex;
