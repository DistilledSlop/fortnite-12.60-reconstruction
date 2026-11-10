// CONFIDENCE: LOW
// Original: 0x1423acc40 (virtual Implementation of ServerSpawnMark). Callees not named here are described in the evidence file.
// Mark list is capped at 4 entries: stale entries are dropped first, then the oldest live mark is released.
AFortPlayerStateZone* PlayerStateZone = Cast<AFortPlayerStateZone>(PlayerState);
SpawnedMarks.RemoveAll([](const TWeakObjectPtr<AFortMarkActor>& Mark) { return !Mark.IsValid(); });
while (SpawnedMarks.Num() > 4)
{
	if (AFortMarkActor* Oldest = SpawnedMarks[0].Get())
	{
		NativeReleaseMark(Oldest, false, true);
	}
	SpawnedMarks.RemoveAt(0);
}
if (!ActorToAttachTo || !ActorToAttachTo->GetWorld())
{
	if (PlayerStateZone)
	{
		UE_LOG(LogFort, Warning, TEXT("Client %s attempted to add mark to invalid actor"), *NativeGetClientName(PlayerStateZone));
	}
	return;
}
// Mark location is the attach root's location plus the offset (zero base when there is no root component).
USceneComponent* Root = ActorToAttachTo->RootComponent;
const FVector Base = Root ? Root->GetComponentLocation() : FVector::ZeroVector;
AFortMarkActor* Mark = Cast<AFortMarkActor>(GetWorld()->SpawnActor(AFortMarkActor::StaticClass(), Base + AttachOffset));
if (Mark)
{
	Mark->AttachOffset = AttachOffset;
	Mark->AttachedActor = ActorToAttachTo;
	Mark->OwnerZone = PlayerStateZone;
	if (!PreapprovedText.IsEmpty())
	{
		Mark->PreapprovedText = PreapprovedText;
		Mark->bHasPreapprovedText = true;
	}
	Mark->FinishSpawning();
	SpawnedMarks.Add(Mark);
}
