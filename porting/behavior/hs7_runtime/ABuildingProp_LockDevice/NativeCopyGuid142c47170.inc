// helper 0x142c47170 (host ABuildingProp_LockDevice): four dword loads from [RDX+0..0xc] stored to [RCX+0..0xc], returns RCX
// evidence: 16-byte copy with RAX = destination = FGuid assignment
// CONFIDENCE: HIGH
*Dest = *Src;
return Dest;
