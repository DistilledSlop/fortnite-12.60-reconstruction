// CONFIDENCE: medium
// Original address: 0x14084c020
// +0xfb0: sub-object pointer (null gives false); +0x3b0: sub-object whose vtable slot 0x670 is called.
uint8* lVar1 = *(uint8**)((uint8*)param_1 + 0xfb0);
if (lVar1 == nullptr)
{
    return false;
}
uint8* pSub = *(uint8**)((uint8*)param_1 + 0x3b0);
float extraout_XMM0_Da = (*(float(**)(void*))((*(uint8**)pSub) + 0x670))(pSub);
return extraout_XMM0_Da <= (float)*(int32*)(lVar1 + 0x550);
