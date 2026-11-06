// CONFIDENCE: medium
// Original address: 0x1440b2120
// Reason: Raw byte offsets 0xa0 and 0xf0 on param_1 and its pointee; layout unnamed.
// raw byte offset 0xa0 on param_1 (pointer field)
if (*(int64*)((uint8*)param_1 + 0xa0) == 0)
{
    return 0;
}
// raw byte offset 0xf0 on the pointee (uint8 field)
return *(uint8*)(*(int64*)((uint8*)param_1 + 0xa0) + 0xf0);
