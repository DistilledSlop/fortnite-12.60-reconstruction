// CONFIDENCE: HIGH
// Original registration retains the integer data pointer, rather than a cached value.
static const TConsoleVariableData<int32>* const WindVariable =
    IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("Fort.Wind"));
return WindVariable->GetValueOnGameThread() != 0;
