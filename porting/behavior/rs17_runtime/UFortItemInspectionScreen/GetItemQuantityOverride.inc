// Original body address: 0x144f260f0 (thunk 0x14526ef70)
// Offset evidence: IsValid (0x142ee7880) on InspectedItem weak at +0x574 (reflected); false -> MOV EAX,-1; true -> dword at +0x648 (unreflected gap, NativeRecoveredInt32_0x648).
// CONFIDENCE: HIGH
if (!InspectedItem.IsValid())
{
    return -1;
}
return NativeRecoveredInt32_0x648;
