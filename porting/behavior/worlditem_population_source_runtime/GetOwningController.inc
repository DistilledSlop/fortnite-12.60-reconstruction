// CONFIDENCE: HIGH
// Original address: 0x1421d8de0.
UWorld* World = GetWorld();
if (World)
{
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        AFortPlayerController* Controller = Cast<AFortPlayerController>(It->Get());
        if (Controller && Controller->McpProfileGroup == ProfileGroup)
        {
            return Controller;
        }
    }
}
return nullptr;
