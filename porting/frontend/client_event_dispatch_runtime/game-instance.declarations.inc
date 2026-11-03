public:
    void NativeCollectClientEventFunctions141e60900(UObject* Object, TSet<FName>& Functions) const;
    void NativeRegisterClientEventObject141e78cb0(UObject* Object);
    void NativeUnregisterClientEventObject141e84a70(UObject* Object);
    void NativeCollectClientEventTargets141e62a90(FName Function, TSet<UObject*>& Targets);
    void NativeExecuteClientEvent141e59c80(const FFortClientEventName& EventType, UObject* Source, UObject* Focus);
