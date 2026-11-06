// CONFIDENCE: HIGH
if (!Controller)
{
    return;
}
static const FString Event(TEXT("Core.ClientUnexpectedSynchronousLoad"));
static const FString File(TEXT("File"));
auto& ProviderSlot = Controller->NativeGetAnalyticsProvider();
FFortNativeAnalyticsProvider* Provider = ProviderSlot.Get();
if (!Provider)
{
    return;
}
TArray<FAnalyticsEventAttribute> Attributes;
Attributes.Reserve(1);
Attributes.Emplace(File, Path);
NativeAppendCoreLoadAttributes(Attributes, Controller->GetWorld());
// The same borrowed shared slot is read again after building attributes.
Provider = ProviderSlot.Get();
FString EventName(Event);
Provider->NativeRecordEvent(EventName, Attributes);
