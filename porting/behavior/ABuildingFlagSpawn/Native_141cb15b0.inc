// CONFIDENCE: medium
// Original address: 0x141cb15b0
uint8* pX = *(uint8**)((uint8*)param_1 + 0x7c0);
if (pX != nullptr)
{
    uint8* pY = *(uint8**)(pX + 0x218);
    uint8* lVar1 = (uint8*)(*(void*(**)(void*))((*(uint8**)pY) + 0x10))(pY);
    if (lVar1 != (uint8*)param_1)
    {
        return 1;
    }
}
if (*(uint8**)((uint8*)param_1 + 0x7c0) == nullptr)
{
    return 1;
}
return 0;
