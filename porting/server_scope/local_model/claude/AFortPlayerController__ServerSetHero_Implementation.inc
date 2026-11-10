// CONFIDENCE: MEDIUM
// Original: 0x1423ac4d0 (virtual Implementation of ServerSetHero). Callees not named here are described in the evidence file.
// Hero selection. A known hero is applied at once; an unknown ID is stored as pending and retried on a 0.1 s timer.
UFortHero* Hero = NativeFindHeroByID(NewHeroID);
if (Hero && Hero->IsA(UFortHero::StaticClass()))
{
	PendingHeroID.Reset();
	FTimerManager& TimerManager = GetWorldTimerManager();
	if (TimerManager.IsTimerActive(HeroRetryTimerHandle))
	{
		TimerManager.ClearTimer(HeroRetryTimerHandle);
	}
	HeroRetryTimerHandle.Invalidate();
	NativeApplyHero(Hero, /*bForce*/ true, /*bLoadout*/ false);
	return;
}
UE_LOG(LogFort, Warning, TEXT("ServerSetHero - Invalid HeroID passed to server: %s"), *NewHeroID);
PendingHeroID = NewHeroID;
// Retry target is an unresolved delegate; the retry runs as a timer after 0.1 seconds, not looping.
GetWorldTimerManager().SetTimer(HeroRetryTimerHandle, FTimerDelegate::CreateUObject(this, &AFortPlayerController::NativeRetryPendingHero), 0.1f, false);
