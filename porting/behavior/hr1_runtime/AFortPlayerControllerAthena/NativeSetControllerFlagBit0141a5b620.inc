// Helper 0x141a5b620 on AFortPlayerControllerAthena (caller ThankBusDriver/ServerThankBusDriver path via 0x1419f3650).
// Asm: AND byte [RCX+0xfdd],0xfe; OR byte [RCX+0xfdd],DL; RET. The OR uses the full DL, reproduced as written.
// No reflected member covers 0xfdd, so a raw byte access is used. All instructions accounted for.
// CONFIDENCE: HIGH
uint8* Raw = reinterpret_cast<uint8*>(Context);
uint8* FlagByte = Raw + 0xfdd;
*FlagByte = static_cast<uint8>((*FlagByte & 0xfe) | Value);
