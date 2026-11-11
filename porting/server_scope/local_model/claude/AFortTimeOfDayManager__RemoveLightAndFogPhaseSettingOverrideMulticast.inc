// CONFIDENCE: MEDIUM. Multicast body: removes the light-and-fog override for one day phase with a blend-out. When the phase is not current or has no override, the phase is refreshed and nothing else happens. A zero or negative blend length (other than -1) finishes at once. Otherwise the phase and blend length are stored for the blend to run.
// Original addresses: exec 0x142b0f770 (thunk; calls the multicast body through virtual slot 0x628). Body 0x141f6bc30. Helpers: 0x141f5c190 (phase refresh and blend finish).

if (CurrentDayNightPhase != DayPhase || LightAndFogPhaseSettingOverrides[DayPhase] == nullptr)
{
    RefreshLightAndFogPhase(DayPhase);
    return;
}

if (bBlendingOutLightAndFogOverrides)
{
    if (PendingBlendPhase != 4 && PendingBlendPhase != DayPhase)
    {
        FinishLightAndFogBlend();
        PendingBlendPhase = 4;
    }
}

bBlendingInLightAndFogOverrides = false;
bBlendingOutLightAndFogOverrides = true;

if (BlendOutLength <= 0.0f && BlendOutLength != -1.0f)
{
    FinishLightAndFogBlend(DayPhase);
    bBlendingOutLightAndFogOverrides = false;
    LightAndFogBlendLength = 0.0f;
    return;
}

PendingBlendPhase = DayPhase;
LightAndFogBlendLength = BlendOutLength;
