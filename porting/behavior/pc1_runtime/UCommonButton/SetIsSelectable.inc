// Original body 0x1407f7e40 (thunk 0x14080d080). Offset evidence: byte [+0x2a0] bit 0x02 = bSelectable (bool_bits, UCommonButton); byte [+0x2a1] bit 0x01 = NativeRecoveredBits_0x2a1 (unreflected gap); tail-jump 0x1407f9880 = SetSelectedInternal(bInSelected=false, bAllowSound=true, bBroadcast=true).
// CONFIDENCE: HIGH
if (bSelectable == bInIsSelectable)
{
    return;
}
bSelectable = bInIsSelectable;
if ((NativeRecoveredBits_0x2a1 & 0x01) == 0)
{
    return;
}
if (bInIsSelectable)
{
    return;
}
SetSelectedInternal(false, true, true);
