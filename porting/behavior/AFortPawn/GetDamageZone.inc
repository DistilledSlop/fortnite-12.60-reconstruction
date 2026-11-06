// CONFIDENCE: high. Native target 0x14231aa10.
if (!InHitResult.BoneName.IsNone())
{
    for (int32 Index = 0; Index < 4; ++Index)
    {
        if (DamageZones[Index].bActive && DamageZones[Index].Bones.Contains(InHitResult.BoneName))
        {
            return static_cast<EFortDamageZone::Type>(Index);
        }
    }
}
return static_cast<EFortDamageZone::Type>(1);
