// CONFIDENCE: HIGH. Original141a02b20; MAXSS chooses the existing timer on unordered/equal values.
Super::UpdateFromCompressedFlags(Flags);
if (CharacterOwner->GetLocalRole() == ROLE_Authority && (Flags & 0x80))
{
    const float Duration = static_cast<AFortPlayerPawnAthena*>(CharacterOwner)->MeleeCombatSlowDuration;
    NativeMeleeSlowRemaining1e5c = Duration > NativeMeleeSlowRemaining1e5c
        ? Duration : NativeMeleeSlowRemaining1e5c;
}
