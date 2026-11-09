// CONFIDENCE: medium
// Original address: 0x1407cfb90
// Raw offsets: +0x1f0 holds a pointer; flag byte +0x409 of that object.
uint64 lVar1 = *(uint64*)((uint8*)param_1 + 0x1f0);
uint64 uVar2 = (lVar1 >> 8) & 0x00ffffffffffffffULL; // uint7 truncation
if ((lVar1 == 0) || (*(uint8*)((uint8*)lVar1 + 0x409) == 0))
{
    return uVar2 << 8;
}
return (uVar2 << 8) | 1; // CONCAT71(uVar2, 1)
