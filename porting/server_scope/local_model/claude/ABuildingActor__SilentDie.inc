// CONFIDENCE: MEDIUM. Kills the building without the normal death presentation, using a silent death reason. The bPropagateSilentDeath flag selects the propagated variant. No-op when already destroyed.
// Original address: 0x142738880 (exec). Native body: 0x141bc5d60 (virtual slot 0x868) which calls NativeDie (virtual slot 0x860) with a tag from the death-reason table.
if (bDestroyed)
{
    return;
}

const FFortDeathReason& SilentReason = bPropagateSilentDeath ? GetSilentDeathPropagatedReason() : GetSilentDeathReason();
NativeDie(SilentReason, nullptr, nullptr, nullptr);
