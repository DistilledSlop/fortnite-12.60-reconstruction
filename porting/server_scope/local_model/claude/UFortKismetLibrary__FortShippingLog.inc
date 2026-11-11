// CONFIDENCE: MEDIUM
// Native 0x14296b950 -> 0x141eef630: writes InString to the log (LogBlueprintUserMessages) at Warning verbosity when
// bLogAsWarning is set, otherwise at Log verbosity. On a client world the text is tagged with the PIE instance
// ("Server: " for instances 1-2, "Client N: " for instance 3). With Kismet.bLogPrintStringSource the world context
// name is prepended.
FString Message = InString;
const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
if (World && World->GetNetMode() == NM_Client)
{
    const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(World);
    const int32 PIEInstance = WorldContext ? WorldContext->PIEInstance : 0;
    if (PIEInstance >= 1 && PIEInstance < 3)
    {
        Message = FString::Printf(TEXT("Server: %s"), *Message);
    }
    else if (PIEInstance == 3)
    {
        Message = FString::Printf(TEXT("Client %d: %s"), GPlayInEditorID - 1, *Message);
    }
}
static const IConsoleVariable* CVarLogPrintStringSource = IConsoleManager::Get().FindConsoleVariable(TEXT("Kismet.bLogPrintStringSource"));
if (CVarLogPrintStringSource && CVarLogPrintStringSource->GetBool() && WorldContextObject)
{
    Message = FString::Printf(TEXT("[%s] %s"), *WorldContextObject->GetName(), *Message);
}
if (bLogAsWarning)
{
    UE_LOG(LogBlueprintUserMessages, Warning, TEXT("%s"), *Message);
}
else
{
    UE_LOG(LogBlueprintUserMessages, Log, TEXT("%s"), *Message);
}
