// CONFIDENCE: high; native 0x144b38c80 resolves the context world and calls the engine replay predicate.
{
    const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    return World && World->IsPlayingReplay();
}
