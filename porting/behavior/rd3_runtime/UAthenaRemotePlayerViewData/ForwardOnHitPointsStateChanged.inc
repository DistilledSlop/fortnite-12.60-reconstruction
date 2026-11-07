// Behaviour snippet: UAthenaRemotePlayerViewData::ForwardOnHitPointsStateChanged (body only; spliced into the generated member function).
// Original body: 0x144c1ff60 (thunk 0x1451b84a0). Offset evidence: cached health +0x20c, cached max health +0x210, cached shield +0x214, cached max shield +0x218 (float gaps, member_field). Delegates OnHealthHealed +0x88, OnHealthDamaged +0x98, OnShieldHealed +0xa8, OnShieldDamaged +0xb8 are FTwoFloatDelegate members, each broadcast with packed {this, float, float}. The compare flags follow COMISS semantics: a greater-than test selects the heal delegate, and a not-greater-or-equal test (less or unordered) selects the damage delegate. InPlayerState is not read by the asm.
// CONFIDENCE: MEDIUM
if (InHealth > NativeRecoveredFloat_0x20c)
{
    OnHealthHealed.Broadcast(this, InHealth, InHealthMax);
}
else if (!(InHealth >= NativeRecoveredFloat_0x20c))
{
    OnHealthDamaged.Broadcast(this, InHealth, InHealthMax);
}
NativeRecoveredFloat_0x210 = InHealthMax;
const float PreviousShield = NativeRecoveredFloat_0x214;
NativeRecoveredFloat_0x20c = InHealth;
if (InShield > PreviousShield)
{
    OnShieldHealed.Broadcast(this, InShield, InShieldMax);
}
else if (!(InShield >= PreviousShield))
{
    OnShieldDamaged.Broadcast(this, InShield, InShieldMax);
}
NativeRecoveredFloat_0x218 = InShieldMax;
NativeRecoveredFloat_0x214 = InShield;
