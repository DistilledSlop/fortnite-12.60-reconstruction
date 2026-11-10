// CONFIDENCE: MEDIUM
// Original: 0x1423ac7a0 (virtual Implementation of ServerSetMarkText). Callees not named here are described in the evidence file.
// Edit the text of a mark owned by this player's zone. Empty text removes the mark.
AFortPlayerStateZone* PlayerStateZone = Cast<AFortPlayerStateZone>(PlayerState);
if (MarkActor && MarkActor->GetWorld() && NativeIsMarkOwnedBy(MarkActor, PlayerStateZone))
{
	if (MarkText.IsEmpty())
	{
		NativeReleaseMark(MarkActor, false, true);
		return;
	}
	MarkActor->MarkText = MarkText;
	MarkActor->bHasCustomText = true;
	return;
}
if (PlayerStateZone)
{
	UE_LOG(LogFort, Warning, TEXT("Client %s attempted to edit invalid mark %s"),
		*NativeGetClientName(PlayerStateZone), MarkActor ? *MarkActor->GetName() : TEXT("None"));
}
