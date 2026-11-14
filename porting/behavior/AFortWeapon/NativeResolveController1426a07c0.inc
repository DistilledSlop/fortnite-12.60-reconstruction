// CONFIDENCE: HIGH. Original1426a07c0; the instigator read precedes even the successful-controller return.
AController* Result = GetInstigatorController();
APawn* InstigatingPawn = GetInstigator();
if (!Result)
{
    if (AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(InstigatingPawn))
    {
        if (PlayerPawn->ControlledRCPawn.IsValid())
        {
            AFortRemoteControlledPawnAthena* RemotePawn = PlayerPawn->ControlledRCPawn.Get();
            Result = RemotePawn->GetController();
            if (Result)
            {
                return Result;
            }
        }
    }
    if (AController* OwnerController = Cast<AController>(GetOwner()))
    {
        return OwnerController;
    }
    if (GetWorld())
    {
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (AFortPlayerControllerSpectating* Spectator = Cast<AFortPlayerControllerSpectating>(It->Get()))
            {
                if (Spectator->NativeGetSpectatedPawn1423d4240(true) == InstigatingPawn)
                {
                    return Spectator;
                }
            }
        }
    }
}
return Result;
