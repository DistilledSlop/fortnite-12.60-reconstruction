// CONFIDENCE: HIGH
if (GetNetMode() == NM_DedicatedServer || !NativeIsWindEnabled() || !NativeWindSamplingInitialized_0x44f)
{
    // The original failure path still consumes a new handle UID.
    return FFortWindImpulseHandle();
}
const int32 Index = WindImpulses.Add(WindImpulse);
return WindImpulses[Index].Handle;
