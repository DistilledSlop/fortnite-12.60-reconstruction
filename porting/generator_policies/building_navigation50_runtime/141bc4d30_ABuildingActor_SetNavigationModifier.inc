// CONFIDENCE: HIGH
// Native 0x141bc4d30; reflected exec 0x142738450.
if (bIsNavigationModifier != bEnableModifier)
{
    bIsNavigationModifier = bEnableModifier;
    UNavigationSystemV1::UpdateActorInNavOctree(*this);
}
