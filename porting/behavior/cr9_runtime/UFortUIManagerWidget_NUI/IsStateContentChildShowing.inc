// Original body 0x14515ae20 (thunk 0x145314fb0). Offset evidence: this+0x338 is StateContentLayer (reflected Object) and is the UObject* Context of NativeHelper143d5a2f0 (0x143d5a2f0). this+0x318 is a byte with no reflected member, so it uses the gap member NativeRecoveredUint8_0x318.
// CONFIDENCE: HIGH
if (UFortAttributeList::NativeHelper143d5a2f0(StateContentLayer) > 0)
{
    return NativeRecoveredUint8_0x318 != 0;
}
return false;
