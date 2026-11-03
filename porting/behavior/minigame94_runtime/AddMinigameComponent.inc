// CONFIDENCE: medium; native 0x142081bd0. Dependencies documented in README.md.
Minigame94_LogComponentAttempt(Component);
if (!Component || Minigame94_IsPendingKill(Component))
{
    return;
}
if (MinigameComponents.Contains(Component))
{
    return;
}
Minigame94_LogComponentAdded(Component);
MinigameComponents.Add(Component);
AActor* Owner = Component->GetOwner();
if (GetLocalRole() == ROLE_Authority && Owner &&
    Minigame94_OwnerHasInitializationState(Owner) &&
    Minigame94_ComponentFlag84Bit3(Component) &&
    Component->bIsRelevantForMinigameInitialization)
{
    ++NumMinigameComponentsServer;
}
Minigame94_OnComponentAdded(this, Component); // native 0x1429bb220
