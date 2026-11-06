// CONFIDENCE: HIGH
// This routine receives a valid support state and its borrowed subsystem.
if (ContextObject.Get())
{
    return ContextTimer;
}
Subsystem->GetInstanceName();
UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
UWorld* World = GameEngine ? GameEngine->GetGameWorld() : nullptr;
return World ? &World->GetTimerManager() : nullptr;
