// CONFIDENCE: MEDIUM; thunk 0x142a7f950: validate (vcall 0xe28) is the return-true stub 0x1406801d0. Impl 0x14234a9e0 treats the 16-byte Attachment struct: a nonzero first qword means add, zero means remove. Add/remove are allowed only for the locally controlled pawn or on authority (vcall 0x6b0 and Role at this+0xf0). Helpers 0x142359070 (123 lines) and 0x1440d4a00 were read only in part, and 0x142a775e0 is a ProcessEvent wrapper that takes the 16-byte struct.
if (Attachment.NativeRecovered_0x0 != 0)
{
	const bool bLocallyControlled = IsLocallyControlled();
	if (!bLocallyControlled && Role != ROLE_Authority)
	{
		// Log: client tried to add an attachment to someone else's pawn (verbosity gated).
		return;
	}
	if (!Mesh)
	{
		// Log: cannot add attachment to a pawn without a mesh (verbosity gated).
		return;
	}
	AttachmentMesh = Attachment;
	ApplyAttachmentMesh();   // FUN_142359070
	if (Role == ROLE_Authority)
	{
		return;
	}
	FRepFortMeshAttachment Snapshot = AttachmentMesh;
	ProcessEventAttachmentChanged(Snapshot);   // FUN_142a775e0
	return;
}
if (AttachmentMesh.NativeRecovered_0x0 == 0)
{
	return;
}
if (IsLocallyControlled() || Role == ROLE_Authority)
{
	AttachmentMesh = FRepFortMeshAttachment();
	if (AttachmentMeshComponent)
	{
		DetachAttachmentMeshComponent();   // FUN_1440d4a00 plus flag 0x20000000 on the component's internal object flags
		AttachmentMeshComponent = nullptr;
	}
	if (Role == ROLE_Authority)
	{
		return;
	}
	FRepFortMeshAttachment Snapshot = AttachmentMesh;
	ProcessEventAttachmentChanged(Snapshot);
}
else
{
	// Log: client tried to remove an attachment from someone else's pawn (verbosity gated).
}
