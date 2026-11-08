// Helper 0x142429d20 on AFortPlayerControllerOutpost (caller ToggleOwnerInPIE 0x142558710 via tail path).
// Asm: AND dword [RCX+0x330],0xfffffffe; MOVZX EAX,DL; OR dword [RCX+0x330],EAX; RET.
// No reflected member covers 0x330 for this class, so a raw dword access is used. All instructions accounted for.
// CONFIDENCE: HIGH
uint8* Raw = reinterpret_cast<uint8*>(Context);
uint32* FlagDword = reinterpret_cast<uint32*>(Raw + 0x330);
*FlagDword = (*FlagDword & 0xfffffffeu) | static_cast<uint32>(Value);
