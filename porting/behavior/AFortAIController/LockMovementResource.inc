// CONFIDENCE: HIGH; captured native resource dispatch.
if (UPathFollowingComponent* PathFollowingComponent = GetPathFollowingComponent())
{
    PathFollowingComponent->LockResource(EAIRequestPriority::HardScript);
}
