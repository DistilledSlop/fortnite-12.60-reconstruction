// CONFIDENCE: medium
// Original address: 0x141ba76d0
float fVar1 = (*(float(**)(void*))((*(uint8**)param_1) + 0x7b0))(param_1);
float fVar2 = (*(float(**)(void*))((*(uint8**)param_1) + 0x7a8))(param_1);
fVar1 = fVar1 - fVar2;
if (fVar1 <= 0.0)
{
    fVar1 = 0.0;
}
return fVar1;
