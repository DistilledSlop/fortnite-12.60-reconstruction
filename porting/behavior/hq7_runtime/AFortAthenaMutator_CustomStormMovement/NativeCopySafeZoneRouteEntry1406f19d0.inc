// Helper 0x1406f19d0 (AFortAthenaMutator_CustomStormMovement), asm porting/generated/native107-impl/0x1406f19d0.asm.
// Copies a 32-byte record: bytes [0,4) and [4,8) as int32, [8,0x18) as 16 bytes (MOVUPS), [0x18,0x20) as int64.
// Every byte is copied exactly once, so this is a plain 32-byte copy from Src to Dest.
// Callers OnRep_SafeZoneRouteIndex (0x141ae0ad0) and OnPlaylistLoaded (0x141adf9e0) pass the destination in RCX and source in RDX.
// CONFIDENCE: HIGH
FMemory::Memcpy(Dest, Src, 0x20);
