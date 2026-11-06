// CONFIDENCE: HIGH
if (GetNetMode() == NM_DedicatedServer || !NativeIsWindEnabled() || !NativeWindSamplingInitialized_0x44f)
{
    return;
}
for (FFortWindImpulseRadius& Existing : WindImpulses)
{
    if (Existing.Handle.UID == ImpulseHandle.UID)
    {
        Existing.PreviousMagnitude = Existing.CurrentMagnitude;
        Existing.Magnitude = 0.0f;
        Existing.BlendTime = BlendTime;
        Existing.CurrentBlendTime = BlendTime;
        return;
    }
}
UE_LOG(LogFortWind, Warning, TEXT("WindImpulse (%d) not found in list when trying to RemoveWindImpulse."), ImpulseHandle.UID);
