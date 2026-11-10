// CONFIDENCE: MEDIUM. Original exec 0x142a5cde0 (RPC thunk, one bool parameter bClientLevelStreamingState).
// Validate is virtual slot 0x2768 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x2770 -> 0x1419f3770, inlined below.
// The delegate is the controller's level-streaming handshake list at +0x2d70 (entries are 16 bytes: listener pointer and a valid flag). Listener virtual slot 0x50 (IsHandshakeReady, name unresolved).
if (Role != ROLE_Authority)
{
	return;
}
LevelStreamRequestHandshakeState.bHandshakeComplete = bClientLevelStreamingState; // controller+0x2d88
if (!bClientLevelStreamingState)
{
	return;
}
HandshakeDelegate.LockCount += 1; // controller+0x2d84
bool bAnyListenerNotReady = false;
for (int32 Index = HandshakeDelegate.Count - 1; Index >= 0; --Index) // controller+0x2d78
{
	const FHandshakeEntry& Entry = HandshakeDelegate.Entries[Index]; // controller+0x2d70, stride 16
	if (Entry.bValid == false || Entry.Listener == nullptr || !Entry.Listener->IsHandshakeReady_Slot0x50())
	{
		bAnyListenerNotReady = true;
	}
}
HandshakeDelegate.LockCount -= 1;
if (bAnyListenerNotReady)
{
	HandshakeDelegate.CompactInvocationList(false); // FMulticastDelegateBase_CompactInvocationList
}
