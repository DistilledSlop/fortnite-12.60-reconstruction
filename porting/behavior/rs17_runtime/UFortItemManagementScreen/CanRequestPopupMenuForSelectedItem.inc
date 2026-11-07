// Original body address: 0x144f54e30 (thunk 0x145273c10)
// Offset evidence: byte Mode at +0x520 must be 0 (CMP byte [RCX+0x520],0). FrontendInventoryFilter byte at +0x521: LEA EAX,[RDX-1]; CMP AL,4; JBE false; CMP DL,9; JNC false; else true.
// CONFIDENCE: HIGH
if (static_cast<uint8>(Mode) != 0)
{
    return false;
}
const uint8 Filter = static_cast<uint8>(FrontendInventoryFilter);
if (static_cast<uint8>(Filter - 1) <= 4)
{
    return false;
}
return Filter < 9;
