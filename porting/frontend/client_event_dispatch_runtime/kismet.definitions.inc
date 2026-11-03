void UFortKismetLibrary::ExecuteClientEvent(UObject* WorldContextObject, FFortClientEventName EventType,
    UObject* EventSource, UObject* EventFocus)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    UFortGameInstance* Instance = World ? Cast<UFortGameInstance>(World->GetGameInstance()) : nullptr;
    if (Instance)
        Instance->NativeExecuteClientEvent141e59c80(EventType, EventSource, EventFocus);
    else
        NativeLogClientEventWorldFailure141eece7a();
}
