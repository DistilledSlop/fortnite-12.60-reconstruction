// CONFIDENCE: medium
// Original address: 0x141fc7d50
if (*(int32*)((uint8*)param_1 + 0x268) != 0)
{
    if (*(int32*)((uint8*)param_1 + 0x390) + 1 != *(int32*)((uint8*)param_1 + 0x3a0))
    {
        return 0;
    }
}
return 1;
