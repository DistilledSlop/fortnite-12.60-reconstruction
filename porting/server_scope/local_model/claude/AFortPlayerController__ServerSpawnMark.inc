// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4d9b0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423acc40.
// Read order: object (offset 0), FVector (offset 8, 12 bytes), FText (offset 24).
if (!ServerSpawnMark_Validate(ActorToAttachTo, AttachOffset, PreapprovedText))
{
	RPC_ValidateFailed(TEXT("ServerSpawnMark_Validate"));
	return;
}
ServerSpawnMark_Implementation(ActorToAttachTo, AttachOffset, PreapprovedText);
