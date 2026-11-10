// CONFIDENCE: MEDIUM
// Original target 0x1419f1fb0 is a jump thunk to FUN_1417af520 (body below).
// One-time lazy init of the event key: "Creative.Loading.MaxWaitingTimeForDeviceReplicationReached".
static FString EventKey = TEXT("Creative.Loading.MaxWaitingTimeForDeviceReplicationReached");
// Telemetry sink and context come from the controller's connection (vslot 0x28 on the component at +0x5a0).
TelemetrySink* Sink = nullptr;
TSharedPtr<FTelemetryContext> Context;
if (this != nullptr)
{
	auto Pair = ConnectionComponent_0x5a0->GetTelemetrySinkAndContext();	// vslot 0x28
	Sink = Pair.Sink;
	Context = Pair.Context;	// added reference, released at the end
}
if (Sink != nullptr)
{
	// Pass the controller only when it IsA AFortPlayerControllerAthena.
	AFortPlayerControllerAthena* Controller = this;
	if (Controller == nullptr || !Controller->IsA(AFortPlayerControllerAthena::StaticClass()))
	{
		Controller = nullptr;
	}
	// FUN_14176cd10 builds the event attributes for the controller (not read beyond depth one).
	TArray<FAnalyticsAttribute> Attributes = BuildEventAttributes(Controller, 0);
	// vslot 0xc8: SendEvent(EventKey, Attributes).
	Sink->SendEvent(EventKey, Attributes);
}
