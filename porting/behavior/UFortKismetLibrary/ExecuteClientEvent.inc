// CONFIDENCE: MEDIUM
// Migrated from the hand-written UFortKismetLibrary in FortniteMapClasses.cpp.
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
UFortGameInstance* Instance = World ? Cast<UFortGameInstance>(World->GetGameInstance()) : nullptr;
if (Instance)
    Instance->NativeExecuteClientEvent141e59c80(EventType, EventSource, EventFocus);
else
    NativeLogClientEventWorldFailure141eece7a();
