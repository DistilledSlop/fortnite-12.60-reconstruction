// Helper 0x14098bd50 (39 insns): RCX = &Dest.SharedReferenceCount, RDX = &Source.SharedReferenceCount.
// Caller evidence (0x142cb9f50 around 0x142cb9fff): MOV [R14],RAX (object pointer), LEA RCX,[R14+0x8], LEA RDX,[RSP+0x28],
// CALL: the helper handles the controller half of TSharedPtr<T, ThreadSafe> move assignment.
// Asm: if ([RDX] != [RCX]) { Old = [RCX]; [RCX] = [RDX]; [RDX] = 0; release Old }. Release = LOCK XADD -1 on [Old+8]; when
// it was 1 call vtable[0] (DestroyObject); LOCK XADD -1 on [Old+0xc]; when it was 1 call vtable[1] (DestroySelf).
// That is FSharedReferencer<ThreadSafe> destruction, expressed with the engine type (layout: one controller pointer).
// CONFIDENCE: MEDIUM
using FThreadSafeReferencer = SharedPointerInternals::FSharedReferencer<ESPMode::ThreadSafe>;
void* const DestController = *reinterpret_cast<void* const*>(Dest);
void* const SourceController = *reinterpret_cast<void* const*>(Source);
if (DestController != SourceController)
{
    FThreadSafeReferencer TakenReference(MoveTemp(*reinterpret_cast<FThreadSafeReferencer*>(Source)));
    FThreadSafeReferencer ReleasedReference(MoveTemp(*reinterpret_cast<FThreadSafeReferencer*>(Dest)));
    ::new (static_cast<void*>(Dest)) FThreadSafeReferencer(MoveTemp(TakenReference));
    // ReleasedReference goes out of scope here and drops the previous controller reference.
}
return Dest;
