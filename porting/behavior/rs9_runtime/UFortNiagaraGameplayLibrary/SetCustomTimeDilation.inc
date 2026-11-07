// Original body: 0x141dd8d80 (thunk 0x142a01700)
// Offset evidence: UParticleSystemComponent reflects CustomTimeDilation Float at +0x4d8. Class tests use IsA against UNiagaraComponent::StaticClass (candidate 0x1409a7e70) and UParticleSystemComponent::StaticClass (candidate 0x1447ad8f0); Niagara targets exit without a store, particle-system targets receive the store.
// CONFIDENCE: MEDIUM
if (Target == nullptr)
{
    return;
}
if (Target->IsA(UNiagaraComponent::StaticClass()))
{
    return;
}
if (!Target->IsA(UParticleSystemComponent::StaticClass()))
{
    return;
}
static_cast<UParticleSystemComponent*>(Target)->CustomTimeDilation = Dilation;
