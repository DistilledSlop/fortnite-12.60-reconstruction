// CONFIDENCE: MEDIUM
// Original addresses: client RPC thunk 0x14290cba0 (reads EndOfDayRecap, virtual slot 0x740 on this, then destroys the
// recap's TArray members) -> _Implementation 0x141e94750 (owner AFortGameState). The implementation takes only `this`,
// so the EndOfDayRecap value is not read by the body. The recap struct is a by-value param destroyed after the call.
EndOfDayRecapDepth += 1;                                  // this+0x344
bool bNeedsCompact = false;
for (int32 Index = EndOfDayListeners.Num() - 1; Index >= 0; --Index)   // this+0x330, 16-byte entries
{
	FDelegateListener& Entry = EndOfDayListeners[Index];
	// A listener is bad when it is unbound (+8 == 0), its pointer is null, or its vtable slot 0x50 reports it invalid.
	if (Entry.IsUnbound() || Entry.Listener == nullptr || !Entry.Listener->IsListenerValid())
	{
		bNeedsCompact = true;
	}
}
EndOfDayRecapDepth -= 1;
if (bNeedsCompact)
{
	FMulticastDelegateBase_CompactInvocationList(EndOfDayListeners, /*bAllowShrink*/ false);   // engine helper
}
