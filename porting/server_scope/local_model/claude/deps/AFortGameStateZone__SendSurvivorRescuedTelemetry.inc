// dep AFortGameStateZone__SendSurvivorRescuedTelemetry original 0x141811c50
// CONFIDENCE: LOW for the analytics and stat plumbing, MEDIUM for the gating. Original 0x141811c50. Rescuer may be null.
TSharedPtr<IAnalyticsProvider> Provider;
if (Rescuer != nullptr)
{
	Provider = Rescuer->AnalyticsSubobject->GetAnalyticsProvider();   // vtable +0x28 on Rescuer+0x5a0; takes a shared reference
}
if (Provider.IsValid())
{
	TArray<FAnalyticsEventAttribute> Attributes;
	Attributes.Emplace(TEXT("SurvivorName"), SurvivorName);
	Provider->RecordEvent(TEXT("Survivor.Rescued"), Attributes);   // vtable +0x58
}
if (Rescuer != nullptr && Rescuer->StatTracker != nullptr)   // Rescuer+0x1a40
{
	// Stat counter path (0x142cd4ab0 name build, 0x14249ee10 counter lookup, 0x1424b3430 add 1.0). Engine stat helpers, not read in full.
	Rescuer->StatTracker->GetCounter(SurvivorRescuedStatName, 4)->Add(1.0);
}
