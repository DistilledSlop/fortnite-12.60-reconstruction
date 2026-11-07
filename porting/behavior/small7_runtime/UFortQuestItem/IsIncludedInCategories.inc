// Original body address: 0x1424470b0
// Offset evidence: pointer at object+0x88 (reflected ItemDefinition). Null returns false. Else bit 0x8 of byte at pointee+0x38a. Pointee class is not reflected, so the byte is read raw.
// CONFIDENCE: MEDIUM
const uint8* Definition = reinterpret_cast<const uint8*>(ItemDefinition);
if (Definition == nullptr) { return false; }
return ((Definition[0x38a] >> 3) & 1) != 0;
