// Helper 0x141cb8820 (UFortGameStateComponent_EventLevel), callers 0x141a7fab0 and 0x141a7fcb0 (camera cut/blend handlers).
// Asm evidence: saves byte [+0xb8], sets it to 1, stores RDX at [+0xb0], loads Target=[+0xa0],
// copies the 16 bytes at R8 to a stack slot, calls Target->vtbl[0x758] with (Target, RDX, &copy),
// then restores byte [+0xb8].
// CONFIDENCE: MEDIUM
uint8* RawComponent_hq10 = reinterpret_cast<uint8*>(Component);
const uint8 PreviousGuard_hq10 = *(RawComponent_hq10 + 0xb8);
*(RawComponent_hq10 + 0xb8) = 1;
*reinterpret_cast<void**>(RawComponent_hq10 + 0xb0) = Context;
UObject* DispatchTarget_hq10 = *reinterpret_cast<UObject* const*>(RawComponent_hq10 + 0xa0);
alignas(16) uint8 PayloadCopy_hq10[16];
FMemory::Memcpy(PayloadCopy_hq10, Payload, 16);
using FForwardDispatchFn_hq10 = void (*)(void*, void*, void*);
void** DispatchVtable_hq10 = *reinterpret_cast<void***>(DispatchTarget_hq10);
reinterpret_cast<FForwardDispatchFn_hq10>(DispatchVtable_hq10[0x758 / sizeof(void*)])(DispatchTarget_hq10, Context, PayloadCopy_hq10);
*(RawComponent_hq10 + 0xb8) = PreviousGuard_hq10;
