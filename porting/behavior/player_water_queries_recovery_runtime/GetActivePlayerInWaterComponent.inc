// CONFIDENCE: HIGH
extern int32 GFortPlayerWaterType;
if (GFortPlayerWaterType == 1)
{
    return NiagaraPlayerStandingInWater;
}
if (GFortPlayerWaterType == 2)
{
    return nullptr;
}
return PSC_PlayerInWater;
