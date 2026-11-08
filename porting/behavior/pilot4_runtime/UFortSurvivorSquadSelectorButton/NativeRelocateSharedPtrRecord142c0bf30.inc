// Helper 0x142c0bf30 (42 insns): RCX = destination record, RDX = source record, returns RCX.
// Asm: [RCX] = [RDX]; [RCX+8] = [RDX+8]; if controller non-null LOCK INC dword [controller+8] (AddSharedReference);
// dword [RCX+0x10] = 0; then the source controller [RDX+8] is released (LOCK XADD -1 on +8, vtable[0], LOCK XADD -1 on +0xc,
// vtable[1]) = FSharedReferencer<ThreadSafe> destruction. The source fields themselves are not cleared.
// CONFIDENCE: MEDIUM
using FThreadSafeReferencer = SharedPointerInternals::FSharedReferencer<ESPMode::ThreadSafe>;
*reinterpret_cast<uint64*>(Dest) = *reinterpret_cast<const uint64*>(Source);
FThreadSafeReferencer* SourceReference = reinterpret_cast<FThreadSafeReferencer*>(Source + 0x8);
::new (static_cast<void*>(Dest + 0x8)) FThreadSafeReferencer(*SourceReference);
*reinterpret_cast<int32*>(Dest + 0x10) = 0;
SourceReference->~FThreadSafeReferencer();
return Dest;
