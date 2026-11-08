// Helper 0x142d5c1a0 (10 callers; e.g. 0x143d51700 passes a 0xb4-byte block in RCX and a bool in DL, then uses RAX).
// Asm: stores zeros into qword/dword/byte fields, 0x80 at +0x44/+0x94, -1 at +0x48/+0x98, byte at +0x11 = DL ^ 1, returns RCX.
// Offset evidence: Block is an unrecorded by-value struct (no reflected class); raw stores with the asm widths.
// CONFIDENCE: MEDIUM
uint8* Block = OutBlock;
*reinterpret_cast<uint64*>(Block + 0x0) = 0;
*reinterpret_cast<uint64*>(Block + 0x8) = 0;
*reinterpret_cast<uint8*>(Block + 0x10) = 0;
*reinterpret_cast<uint8*>(Block + 0x12) = 0;
*reinterpret_cast<uint64*>(Block + 0x18) = 0;
*reinterpret_cast<uint64*>(Block + 0x20) = 0;
*reinterpret_cast<uint64*>(Block + 0x38) = 0;
*reinterpret_cast<uint32*>(Block + 0x40) = 0;
*reinterpret_cast<uint32*>(Block + 0x4c) = 0;
*reinterpret_cast<uint64*>(Block + 0x58) = 0;
*reinterpret_cast<uint32*>(Block + 0x60) = 0;
*reinterpret_cast<uint64*>(Block + 0x68) = 0;
*reinterpret_cast<uint64*>(Block + 0x70) = 0;
*reinterpret_cast<uint64*>(Block + 0x88) = 0;
*reinterpret_cast<uint32*>(Block + 0x90) = 0;
*reinterpret_cast<uint32*>(Block + 0x9c) = 0;
*reinterpret_cast<uint64*>(Block + 0xa8) = 0;
*reinterpret_cast<uint32*>(Block + 0xb0) = 0;
*reinterpret_cast<uint32*>(Block + 0x44) = 0x80;
*reinterpret_cast<uint32*>(Block + 0x48) = 0xffffffffu;
*reinterpret_cast<uint32*>(Block + 0x94) = 0x80;
*reinterpret_cast<uint32*>(Block + 0x98) = 0xffffffffu;
*reinterpret_cast<uint8*>(Block + 0x11) = static_cast<uint8>(bFlag ? 0 : 1);
return Block;
