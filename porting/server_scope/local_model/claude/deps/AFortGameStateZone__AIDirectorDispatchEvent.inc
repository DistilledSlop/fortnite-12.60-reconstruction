// dep AFortGameStateZone__AIDirectorDispatchEvent original 0x1416570e0
// CONFIDENCE: LOW-MEDIUM. Original 0x1416570e0. Listeners are bucketed by event type (TMap at +0x218, bucket head at +0x250/+0x258), each bucket holding a listener array (0x20-byte records).
const FAIDirectorListenerBucket* Bucket = nullptr;
if (ListenersByType.Num() != ListenerBucketCount)   // +0x220 vs +0x24c
{
	Bucket = ListenersByType.FindBucket(Event.Type);
}
if (Bucket != nullptr && Bucket->Listeners.Num() > 0)
{
	int32 Index = 0;
	while (Index < Bucket->Listeners.Num())
	{
		FAIDirectorListener& Listener = Bucket->Listeners[Index];
		const bool bFunctionNone = FName_EqualsEName(Listener.FunctionName, NAME_None);
		UObject* ListenerObject = Listener.Object.Get();   // weak pointer at record +0
		if ((Listener.FunctionName.Number == 0 && bFunctionNone) || ListenerObject == nullptr || ListenerObject->FindFunction(Listener.FunctionName) == nullptr)
		{
			Bucket->Listeners.RemoveAt(Index, 1, /*bAllowShrink*/ true);   // decompile shifts tail left then the loop index still advances
			Bucket->Listeners.Shrink();                                    // 0x140743570
		}
		else
		{
			bool bMatch = false;
			if (Listener.MatchKind == 2)
			{
				bMatch = (Event.Source == Listener.Key || Event.Target == Listener.Key);
			}
			else if (Listener.MatchKind == 1)
			{
				bMatch = (Event.Source == Listener.Key);
			}
			else if (Listener.MatchKind == 0)
			{
				bMatch = (Event.Target == Listener.Key);
			}
			else
			{
				bMatch = true;   // other kinds always deliver
			}
			if (bMatch && !(Listener.FunctionName.Number == 0 && bFunctionNone) && ListenerObject != nullptr)
			{
				FAIDirectorEventParams Params = Event;   // 0x20-byte copy
				ListenerObject->ProcessEvent(ListenerObject->FindFunctionChecked(Listener.FunctionName), &Params);
			}
		}
		++Index;
	}
}
