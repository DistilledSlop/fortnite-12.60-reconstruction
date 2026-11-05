// CONFIDENCE: HIGH (source recovery; build/runtime pending)
if (GetNetMode() == NM_DedicatedServer || !NativeIsWindEnabled())
{
    return false;
}
for (const FFortWindImpulseCylinder& Impulse : WindImpulseCylinders)
{
    if (Impulse.Handle.UID == ImpulseHandle.UID)
    {
        OutImpulse = Impulse;
        return true;
    }
}
UE_LOG(LogFortWind, Warning, TEXT("WindImpulseCylinder (%d) not found in list when trying to GetWindImpulseCylinder."), ImpulseHandle.UID);
return false;
