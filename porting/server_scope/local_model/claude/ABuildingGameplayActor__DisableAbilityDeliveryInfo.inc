// CONFIDENCE: MEDIUM. Chunk 01 port. See ABuildingGameplayActor__DisableAbilityDeliveryInfo.evidence.json for captures and notes.
// Original address: 0x14273fbc0 (exec). Body 0x141bd96a0.
// Only the authority runs this (Role is at +0xf0, value 3 = ROLE_Authority).
if (Role != ROLE_Authority)
{
	return;
}

bool bOtherBucketPulses = false;
for (FBuildingGameplayActorAbilityDeliveryBucket& Bucket : DeliverableAbilityInfo.DeliveryBuckets)
{
	if (Bucket.Tag == BucketTag)
	{
		SetBucketEnabled(Bucket, DeliverableAbilityInfo.OwningActor, false);
	}
	else if (Bucket.bHasGEsToApplyOnPulseTimer)
	{
		bOtherBucketPulses = true;
	}
}

// Stop the proximity pulse timers only when no other enabled bucket still needs them.
ABuildingGameplayActor* Owner = DeliverableAbilityInfo.OwningActor;
if (Owner && Owner->GetWorld() && !bOtherBucketPulses)
{
	FTimerManager& TimerManager = Owner->GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(DeliverableAbilityInfo.ProximityPulseTimerHandle))
	{
		TimerManager.ClearTimer(DeliverableAbilityInfo.ProximityPulseTimerHandle);
	}
	DeliverableAbilityInfo.ProximityPulseTimerHandle.Invalidate();

	if (TimerManager.IsTimerActive(DeliverableAbilityInfo.ProximityPrePulseTimerHandle))
	{
		TimerManager.ClearTimer(DeliverableAbilityInfo.ProximityPrePulseTimerHandle);
	}
	DeliverableAbilityInfo.ProximityPrePulseTimerHandle.Invalidate();
}
