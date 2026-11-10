// CONFIDENCE: MEDIUM. Chunk 01 port. See ABuildingGameplayActor__EnableAbilityDeliveryInfo.evidence.json for captures and notes.
// Original address: 0x14273ff20 (exec). Body 0x141bda910.
if (Role != ROLE_Authority)
{
	return;
}

for (FBuildingGameplayActorAbilityDeliveryBucket& Bucket : DeliverableAbilityInfo.DeliveryBuckets)
{
	if (Bucket.Tag == BucketTag)
	{
		EnableDeliveryBucket(Bucket);
	}
}
