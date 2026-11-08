// Helper 0x144217e60 (UFortMatchmakingContext), 6 instructions:
//   MOV RAX,[RCX+0x30]; TEST RAX,RAX; JZ 0x144217e71; MOV RAX,[RAX+0x278]; RET; 0x144217e71: RET
// Returns the pointer at [Context+0x30] dereferenced at +0x278, or null when [Context+0x30] is null.
// Callers 141cad960 and 14259f540 pass RCX = the context returned by 0x141ca8de0.
// [+0x30] is read as a raw pointer-sized word (its first 8 bytes are not a reflected pointer property);
// the pointee type at +0x278 is not reflected, so it is returned as void*.
// CONFIDENCE: HIGH
const uint8* ContextBytes = reinterpret_cast<const uint8*>(MatchmakingCtx);
void* Inner = *reinterpret_cast<void* const*>(ContextBytes + 0x30);
if (Inner == nullptr)
{
	return nullptr;
}
return *reinterpret_cast<void* const*>(reinterpret_cast<const uint8*>(Inner) + 0x278);
