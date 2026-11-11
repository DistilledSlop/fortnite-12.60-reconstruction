// CONFIDENCE: MEDIUM. Multicast body: installs the light-and-fog setting override for one day phase. Clears the existing override for the phase, rejects classes that do not derive from the day-phase settings base, creates the override under this manager, copies the phase's settings handle into it, and when the phase is the current one starts the blend-in. A zero or negative blend length applies at full strength at once, except -1 which is kept as a manual blend.
// Original addresses: exec 0x142b0e220 (thunk; calls the multicast body through virtual slot 0x630). Body 0x141f579e0. Helpers: 0x141f5c190 (phase refresh), 0x1428cfa30 (day-phase settings base class getter, name unresolved), 0x142ed7360 (StaticConstructObject), 0x141f77a60 (settings refresh), 0x141f5a070 (blend apply).

RefreshLightAndFogPhase(DayPhase);
if (DayPhaseInfoOverride == nullptr)
{
    return;
}

UClass* DayPhaseSettingsBaseClass = GetDayPhaseSettingsBaseClass();
if (DayPhaseSettingsBaseClass == nullptr || !DayPhaseInfoOverride->IsChildOf(DayPhaseSettingsBaseClass))
{
    return;
}

UObject* Override = NewObject<UObject>(this, DayPhaseInfoOverride);
LightAndFogPhaseSettingOverrides[DayPhase] = Override;
if (Override == nullptr)
{
    return;
}

// Per-phase settings block (this + 0x528 + phase * 0x210) provides the handle copied into the override (LOW).
Override->SettingsHandle = PhaseSettings[DayPhase].SettingsHandle;
RefreshLightAndFogPhaseSettings();

if (CurrentDayNightPhase == DayPhase)
{
    bBlendingOutLightAndFogOverrides = false;
    bBlendingInLightAndFogOverrides = true;

    if (BlendInLength > 0.0f || BlendInLength == -1.0f)
    {
        LightAndFogBlendLength = BlendInLength;
        ApplyLightAndFogBlend(LightAndFogBlendProgress);
    }
    else
    {
        ApplyLightAndFogBlend(1.0f);
        LightAndFogBlendLength = 0.0f;
    }
}
