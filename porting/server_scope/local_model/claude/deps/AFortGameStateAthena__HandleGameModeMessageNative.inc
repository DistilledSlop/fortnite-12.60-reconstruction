// Native 0x14199a760. CONFIDENCE: LOW (the delayed path is summarised; the timer helper 0x1446278d0 and message object layout are not ported line by line).
// Negative delay: request is dropped. |Delay| <= 1e-8: enqueued immediately (0x14198aa30 on this+0x217). Positive delay: needs a game-mode owner (virtual slot 0x150); a one-shot delayed message object is built with a creation timestamp and scheduled on that owner's timer manager (owner+0x188 -> +0xc0, else owner+0x320).
if (Request.Delay < 0.0f)
{
	return;
}
if (FMath::Abs(Request.Delay) > 1e-8f)
{
	UObject* Owner = GetGameModeForMessages();
	if (Owner != nullptr)
	{
		FDelayedGameModeMessage* Pending = new FDelayedGameModeMessage(this, Request, GetWorldTimeStamp());
		FDelegateHandle Handle = Pending->MakeDelegate();
		UObject* TimerManager = Owner->TimerManagerRef != nullptr ? Owner->TimerManagerRef->TimerManager : Owner->TimerManagerFallback;
		ScheduleOneShot(TimerManager, Request, Handle, Request.Delay);
		Pending->Destroy();
	}
	return;
}
EnqueueGameModeMessageNow(Request);
