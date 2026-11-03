// CONFIDENCE: medium; native 0x142081a40. Dependencies documented in README.md.
if (!Actor || Minigame94_IsPendingKill(Actor))
{
    return;
}
for (UFortMinigameLogicComponent* Existing : MinigameComponents)
{
    if (Existing && Existing->GetOwner() == Actor)
    {
        return;
    }
}
TArray<UFortMinigameLogicComponent*> Components;
Minigame94_GetActorComponents(Actor, Components); // native class-filtered query
for (UFortMinigameLogicComponent* Component : Components)
{
    AddMinigameComponent(Component);
}
