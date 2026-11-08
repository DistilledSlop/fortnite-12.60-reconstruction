// Original body 0x14506b370 (thunk 0x1452d89c0): pointer at +0x5c0 (NativeRecoveredPointer_0x5c0, unreflected) to a record holding an FString at +0x18 (Data) and Num at +0x20. Returns a copy of that string when the pointer is set and Num != 0, else an empty string.
// CONFIDENCE: MEDIUM
FString EventIdResult;
uint8* EventSource = reinterpret_cast<uint8*>(NativeRecoveredPointer_0x5c0);
if (EventSource != nullptr)
{
    const FString& EventSourceString = *reinterpret_cast<const FString*>(EventSource + 0x18);
    if (EventSourceString.Num() != 0)
    {
        EventIdResult = EventSourceString;
    }
}
return EventIdResult;
