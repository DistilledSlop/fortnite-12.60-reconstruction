// Helper 0x142607f40 (AFortPlayerController), callers 0x141ccfa40 and 0x14238e840 (fullscreen map gates; RCX = controller).
// Asm evidence: if int32 [+0x3c0] == 0 -> false; obj=[+0x3b8], null -> false; obj->vtbl[0x28] (slot 5)(obj) false -> false;
// else return byte [+0x570] != 0 || qword [+0x578] != 0.
// CONFIDENCE: HIGH
const uint8* RawController_hq10 = reinterpret_cast<const uint8*>(Controller);
if (*reinterpret_cast<const int32*>(RawController_hq10 + 0x3c0) == 0)
{
	return false;
}
UObject* GateComponent_hq10 = *reinterpret_cast<UObject* const*>(RawController_hq10 + 0x3b8);
if (GateComponent_hq10 == nullptr)
{
	return false;
}
using FGateQueryFn_hq10 = bool (*)(void*);
void** GateVtable_hq10 = *reinterpret_cast<void***>(GateComponent_hq10);
if (!reinterpret_cast<FGateQueryFn_hq10>(GateVtable_hq10[0x28 / sizeof(void*)])(GateComponent_hq10))
{
	return false;
}
return *(RawController_hq10 + 0x570) != 0 || *reinterpret_cast<void* const*>(RawController_hq10 + 0x578) != nullptr;
