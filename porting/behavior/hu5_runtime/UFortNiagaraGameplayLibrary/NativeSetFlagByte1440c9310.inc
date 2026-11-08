// Helper 0x1440c9310 (host UFortNiagaraGameplayLibrary, static). Clears bit 0 of the byte at +0x86, then ORs the full FlagValue byte in (no masking of FlagValue).
// Evidence: AND byte ptr [RCX+0x86],0xfe; OR byte ptr [RCX+0x86],DL; RET. lookup_offset.py UObject 0x86 reports no reflected member, so raw access is used.
// CONFIDENCE: HIGH
uint8* FlagByte = reinterpret_cast<uint8*>(Object) + 0x86;
*FlagByte = static_cast<uint8>((*FlagByte & 0xFE) | FlagValue);
