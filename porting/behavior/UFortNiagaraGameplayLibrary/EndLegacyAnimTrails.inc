// CONFIDENCE: high; native 0x141dca290 dispatches Deactivate on Niagara and EndTrails on legacy particles.
{
    if (UNiagaraComponent* Niagara = Cast<UNiagaraComponent>(Target))
    {
        Niagara->Deactivate();
    }
    else if (UParticleSystemComponent* Particles = Cast<UParticleSystemComponent>(Target))
    {
        Particles->EndTrails();
    }
}
