// CONFIDENCE: HIGH
extern int32 GFortPlayerWaterType;
if (GFortPlayerWaterType == 1)
{
    return NiagaraPlayerSwimmingInWater;
}
if (GFortPlayerWaterType == 2)
{
    return nullptr;
}
return PSC_PlayerInWaterSurfaceSwimming;
