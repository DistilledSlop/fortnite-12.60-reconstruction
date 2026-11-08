// Helper body for UFortQuestItem static helper at 0x14418a050 (RCX = key A, RDX = key B, returns bool in AL).
// Asm: CMP qword [RCX],[RDX] (+0x0); CMP dword [RCX+0x8] vs [RDX+0x8]; CMP dword [RCX+0xc] vs [RDX+0xc]; TEST AL,R8B; returns 1 only when all three match.
// Bytes +0x4..+0x7 are not compared. Caller: UFortQuestItem::GetObjectiveInfo 0x142440da0 loops with RCX = RSI key, RDX = element +0x18 (stride 0x140).
// Key layout is not reflected, so raw reads with the asm's widths are used.
// CONFIDENCE: HIGH
return *reinterpret_cast<const uint64*>(LeftKey) == *reinterpret_cast<const uint64*>(RightKey)
    && *reinterpret_cast<const int32*>(LeftKey + 0x8) == *reinterpret_cast<const int32*>(RightKey + 0x8)
    && *reinterpret_cast<const int32*>(LeftKey + 0xc) == *reinterpret_cast<const int32*>(RightKey + 0xc);
