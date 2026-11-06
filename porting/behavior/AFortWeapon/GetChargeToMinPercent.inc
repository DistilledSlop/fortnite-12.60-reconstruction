// CONFIDENCE: HIGH; see HANDOFF.md.
const FFortBaseWeaponStats* Stats = NativeGetWeaponStats();
UWorld* World = GetWorld();
if (World && Stats && UFortWeaponItemDefinition::NativeSupportsCharge(*Stats))
{
    float MinCharge, MaxCharge;
    NativeGetChargeRange(MinCharge, MaxCharge);
    if (MinCharge > 0.0f)
    {
        const float Value = ChargeTime / MinCharge;
        return Value >= 0.0f ? (Value < 1.0f ? Value : 1.0f) : 0.0f;
    }
}
return 0.0f;
