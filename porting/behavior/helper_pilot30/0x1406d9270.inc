// CONFIDENCE: medium
// Original address: 0x1406d9270
// Reason: TArray-like layout (data at +0, ArrayMax at +0xc) and element size inferred from size scaling; element type unnamed.
// TArray-style resize, element size 1 bytes (from decompile size scaling)
int32 iVar1 = 0;
if (param_2 != 0)
{
    uint64 uVar2 = FMemory_QuantizeSize((int64)param_2, 0);
    iVar1 = (int32)(uVar2);
    if (iVar1 < param_2)
    {
        iVar1 = 0x7fffffff;
    }
}
if ((iVar1 != param_3) && (((void*)*param_1 != nullptr) || (iVar1 != 0)))
{
    void* puVar3 = FMemory_Realloc((void*)*param_1, (int64)iVar1, 0);
    *param_1 = (uint64)puVar3;
}
*(int32*)((uint8*)param_1 + 0xc) = iVar1;
