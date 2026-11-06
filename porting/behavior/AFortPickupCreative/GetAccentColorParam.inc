// CONFIDENCE: high
// Native target 0x1419d2be0 checks CachedPickupInstigatorHandle != -1 then returns CachedInstigatorData.AccentColorParam.
if (CachedPickupInstigatorHandle == -1)
{
    return 0.0f;
}
return CachedInstigatorData.AccentColorParam;
