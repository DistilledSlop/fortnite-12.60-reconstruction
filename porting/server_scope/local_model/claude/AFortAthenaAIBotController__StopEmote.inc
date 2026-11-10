// CONFIDENCE: MEDIUM. Original exec 0x1427c36c0 (no parameters). The thunk body is inline below.
// The emote-state sub-object is read from PlayerBotPawn+0x4c8 and its vtable slot 0x10 is called to fetch the
// emote tracking object (NativeRecoveredEmoteTracker). Its name is not recovered, so the call is shown as a virtual dispatch.
if (PlayerBotPawn)
{
    UObject* EmoteTracker = NativeRecoveredEmoteSubobject_0x4c8->NativeGetEmoteTracker();
    if (EmoteTracker)
    {
        if (NativeRecoveredObject_0x9e8)
        {
            FMulticastDelegate_RemoveByOwner(EmoteTracker->NativeRecoveredDelegate_0x240, NativeRecoveredObject_0x9e8);
            NativeRecoveredObject_0x9e8 = nullptr;
        }
        if (NativeRecoveredInt32_0x9e0 != -1)
        {
            EmoteTracker->RemoveTrackedSourceId(NativeRecoveredInt32_0x9e0);
            NativeRecoveredInt32_0x9e0 = -1;
        }
    }

    PlayerBotPawn->LastReplicatedEmoteExecuted = nullptr;
    PlayerBotPawn->RefreshEmoteTransform();
}
