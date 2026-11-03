// CONFIDENCE: HIGH
if (!Controller)
{
    return;
}
static const FString Event(TEXT("Athena.ClientUnexpectedAsynchronousLoad"));
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
FString EventName(Event);
Provider->NativeRecordEvent(EventName, NativeAppendAthenaLoadAttributes(MoveTemp(Attributes), Controller->GetWorld()));
