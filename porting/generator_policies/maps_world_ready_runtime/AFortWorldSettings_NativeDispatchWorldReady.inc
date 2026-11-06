// CONFIDENCE: HIGH
while (ActorsNeedingWorldReady.Num() > 0)
{
    // Detach this batch so actors registered by callbacks form the next batch.
    TArray<AActor*> PendingActors = MoveTemp(ActorsNeedingWorldReady);
    for (AActor* Actor : PendingActors)
    {
        if (Actor)
        {
            IFortInitializationInterface* Initialization = Cast<IFortInitializationInterface>(Actor);
            if (Initialization && Initialization->NeedsWorldReady())
            {
                if (!Actor->IsPendingKill())
                    Initialization->WorldReady();
            }
        }
    }
}
