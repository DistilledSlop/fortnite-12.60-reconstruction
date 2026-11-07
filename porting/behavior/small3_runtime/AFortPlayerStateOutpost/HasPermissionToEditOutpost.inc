// Original body: 0x14256f710 (thunk 0x142a8d820)
// Offset evidence: TEST byte [RCX+0x330],1 -> true; CMP byte [RCX+0xd20],0 -> true; else false
// CONFIDENCE: HIGH
return bIsWorldDataOwner || bHasPermissionToEditOutpost;
