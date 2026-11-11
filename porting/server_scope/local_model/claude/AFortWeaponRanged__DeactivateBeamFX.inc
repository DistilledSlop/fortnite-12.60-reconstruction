// CONFIDENCE: MEDIUM - null guard and virtual dispatch on BeamPSC are exact; the slot (+0x270) is taken as UParticleSystemComponent::DeactivateSystem and is not name-confirmed.
// Original address: 0x142b4f280 (dispatch on BeamPSC vtable +0x270 -> 0x14439d410).
if (BeamPSC)
{
    BeamPSC->DeactivateSystem();
}
