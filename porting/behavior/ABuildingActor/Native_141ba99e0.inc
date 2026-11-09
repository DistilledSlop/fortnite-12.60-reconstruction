// CONFIDENCE: medium
// Original address: 0x141ba99e0
float fVar1 = (*(float(**)(void*))((*(uint8**)param_1) + 0x7c8))(param_1);
if (fVar1 <= 0.0)
{
    (*(void(**)(void*))((*(uint8**)param_1) + 2000))(param_1);
    return;
}
return;
