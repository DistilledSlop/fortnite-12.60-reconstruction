// CONFIDENCE: medium
// Original address: 0x14468f240
// Reason: Raw byte offsets 0x1c8 and 0x1d0 on param_1; struct layout unnamed.
// raw byte offset 0x1d0 on param_1 (int32 count)
if (*(int32*)((uint8*)param_1 + 0x1d0) < 1)
{
    return (UObject*)0;
}
// raw byte offset 0x1c8 on param_1 (weak pointer storage)
return FWeakObjectPtr_Get(*(void**)((uint8*)param_1 + 0x1c8));
