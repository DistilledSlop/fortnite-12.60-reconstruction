// 141e60900: ordinary field iteration includes superclasses and deprecated fields.
void UFortGameInstance::NativeCollectClientEventFunctions141e60900(UObject* Object, TSet<FName>& Functions) const
{
    if (!Object || !Object->GetClass()->ImplementsInterface(UFortClientEventInterface::StaticClass()))
        return;
    for (TFieldIterator<UFunction> It(Object->GetClass()); It; ++It)
        if (It->GetName().StartsWith(TEXT("HandleClientEvent_"), ESearchCase::IgnoreCase))
            Functions.Add(It->GetFName());
}

void UFortGameInstance::NativeRegisterClientEventObject141e78cb0(UObject* Object)
{
    TSet<FName> Functions;
    NativeCollectClientEventFunctions141e60900(Object, Functions);
    for (FName Function : Functions)
        NativeClientEventListeners6c0.FindOrAdd(Function).Add(Object);
}

// Removing an object retains the now-empty per-function set, as in the original.
void UFortGameInstance::NativeUnregisterClientEventObject141e84a70(UObject* Object)
{
    TSet<FName> Functions;
    NativeCollectClientEventFunctions141e60900(Object, Functions);
    for (FName Function : Functions)
        if (TSet<TWeakObjectPtr<UObject>>* Listeners = NativeClientEventListeners6c0.Find(Function))
            Listeners->Remove(Object);
}

void UFortGameInstance::NativeCollectClientEventTargets141e62a90(FName Function, TSet<UObject*>& Targets)
{
    if (TSet<TWeakObjectPtr<UObject>>* Listeners = NativeClientEventListeners6c0.Find(Function))
    {
        for (auto It = Listeners->CreateIterator(); It; ++It)
        {
            if (It->IsValid())
                Targets.Add(It->Get());
            else
                It.RemoveCurrent();
        }
    }
}

void UFortGameInstance::NativeExecuteClientEvent141e59c80(const FFortClientEventName& EventType, UObject* Source, UObject* Focus)
{
    const FName Specific(*FString::Printf(TEXT("HandleClientEvent_%s"), *EventType.EventName.ToString()));
    static const FName Fallback(TEXT("HandleClientEvent_Undefined"));
    TSet<UObject*> Targets;
    NativeCollectClientEventTargets141e62a90(Specific, Targets);
    NativeCollectClientEventTargets141e62a90(Fallback, Targets);
    // The original forms a distinct raw target set before any ProcessEvent call.
    // Reentrant registry changes therefore affect the next dispatch.
    for (UObject* Target : Targets)
    {
        if (!Target || !Target->GetClass()->ImplementsInterface(UFortClientEventInterface::StaticClass()))
            continue;
        UFunction* Function = Target->FindFunction(Specific);
        if (!Function)
            Function = Target->FindFunction(Fallback);
        if (Function)
        {
            // Original141e5a070..99 copies source/focus, then the complete
            // 32-byte event (type + source/focus) into its 48-byte parameter frame.
            struct FParameters
            {
                UObject* EventSource;
                UObject* EventFocus;
                FFortClientEventName EventType;
                UObject* PayloadSource;
                UObject* PayloadFocus;
            } Parameters {Source, Focus, EventType, Source, Focus};
            static_assert(sizeof(FParameters) == 48, "original client-event parameter frame");
            Target->ProcessEvent(Function, &Parameters);
        }
    }
}
