// CONFIDENCE: MEDIUM. Chunk 01 port. See ABuildingGameplayActorSentry__SetPawnBeingTracked.evidence.json for captures and notes.
// Original address: 0x142744530 (exec). Body 0x141beaa20.
// Authority only. Stores the tracked pawn and, when the sentry is not already aggroed, starts the aggro delay timer.
if (Role != ROLE_Authority)
{
	return;
}

PawnBeingTracked = InPawnBeingTracked;
if (InPawnBeingTracked == nullptr || CurrentSentryState == ESentryState::Aggroed)
{
	return;
}

UWorld* World = GetWorld();
if (World == nullptr)
{
	return;
}

const float AggroDelay = TrackedToAggroDelay.GetValueAtLevel(0.f);
FTimerDelegate AggroDelegate = FTimerDelegate::CreateUObject(this, &ABuildingGameplayActorSentry::OnTrackedToAggroDelayElapsed);
World->GetTimerManager().SetTimer(AggroTimerHandle, AggroDelegate, AggroDelay, false);
