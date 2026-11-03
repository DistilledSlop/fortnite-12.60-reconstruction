// CONFIDENCE: high; recovered native body and reflected field mapping.
// Original exec 0x140859710; direct implementation in native95-impl.
if (FortHoagieVehicleConfigs && FortHoagieVehicleConfigs->RotorMaxSpeed > 0.0f)
{
    const float Value = CurrentRotorSpeed / FortHoagieVehicleConfigs->RotorMaxSpeed;
    if (Value >= 0.0f)
    {
        return Value >= 1.0f ? 1.0f : Value;
    }
}
return 0.0f;
