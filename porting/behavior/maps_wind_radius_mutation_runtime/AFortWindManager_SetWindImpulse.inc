// CONFIDENCE: HIGH
if (GetNetMode() == NM_DedicatedServer || !NativeIsWindEnabled() || !NativeWindSamplingInitialized_0x44f)
{
    return false;
}
for (FFortWindImpulseRadius& Existing : WindImpulses)
{
    if (Existing.Handle.UID == ImpulseHandle.UID)
    {
        Existing.PreviousMagnitude = Existing.CurrentMagnitude;
        Existing.Magnitude = WindImpulse.Magnitude;
        Existing.BlendTime = WindImpulse.BlendTime;
        Existing.CurrentBlendTime = WindImpulse.BlendTime;
        if (Existing.Location.X != WindImpulse.Location.X || Existing.Location.Y != WindImpulse.Location.Y ||
            Existing.Location.Z != WindImpulse.Location.Z || Existing.Radius != WindImpulse.Radius)
        {
            Existing.Location = WindImpulse.Location;
            const float CurrentRadius = Existing.CurrentRadius;
            Existing.Radius = WindImpulse.Radius;
            Existing.PreviousRadius = CurrentRadius;
            Existing.WorldBounds.Min = FVector(Existing.Location.X - CurrentRadius, Existing.Location.Y - CurrentRadius, Existing.Location.Z - CurrentRadius);
            Existing.WorldBounds.Max = FVector(Existing.Location.X + CurrentRadius, Existing.Location.Y + CurrentRadius, Existing.Location.Z + CurrentRadius);
            Existing.WorldBounds.IsValid = 1;
        }
        return true;
    }
}
UE_LOG(LogFortWind, Warning, TEXT("WindImpulse (%d) not found in list when trying to SetWindImpulse."), ImpulseHandle.UID);
return false;
