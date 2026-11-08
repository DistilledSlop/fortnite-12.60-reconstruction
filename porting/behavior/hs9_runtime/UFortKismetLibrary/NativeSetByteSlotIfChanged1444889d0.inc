// Helper 0x1444889d0 (10 callers; caller 0x141bfbc10 passes &table, constant index 3/4/0xe/0x10 and value 0 in R8D).
// Asm: CMP EDX,0x20; JNC ret false; MOVZX EAX,[RDX+RCX]; CMP EAX,R8D; JZ ret false; MOV [RDX+RCX],R8B; ret true.
// Offset evidence: Slots is a stack byte table (RSP+0x20 in caller); bytes indexed directly, no class members.
// CONFIDENCE: HIGH
if (Index >= 0x20u)
{
	return false;
}
if (static_cast<int32>(Slots[Index]) == NewValue)
{
	return false;
}
Slots[Index] = static_cast<uint8>(NewValue);
return true;
