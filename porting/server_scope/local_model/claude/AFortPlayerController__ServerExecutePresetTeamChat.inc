// CONFIDENCE: LOW. Original exec 0x142a4b490 (RPC thunk). Parameters in read order: SenderID (struct), ChatText (FText).
// Validate is virtual slot 0x1428 -> 0x1423ab340 (18 lines), which is real. Ported literally below. Its return sense is inverted relative to a normal check: it reports valid only when the sender check fails and the text's second field is set. This was read directly from the decompile and is unverified.
// The validate also releases its temporary FText block (0x1409f53f0); that release is not repeated here because the implementation reads the same block afterwards.
// Implementation is virtual slot 0x1430 -> 0x1423ab1a0 (75 lines), inlined below. It forwards the chat to every player controller on the same team.
const bool bSenderCharsValid = AllChatCharactersValid_0x142c2e460(SenderPayload);
bool bValidateResult = false;
if (!bSenderCharsValid)
{
	bValidateResult = ChatText.Field_0x8 != nullptr;
}
if (!bValidateResult)
{
	RPC_ValidateFailed(TEXT("ServerExecutePresetTeamChat_Validate"));
	return;
}
UWorld* World = GetWorld();
for (AActor* Entry : World->PlayerControllerList_0x1c8)
{
	AFortPlayerController* Target = Cast<AFortPlayerController>(Entry);
	if (Target == nullptr)
	{
		continue;
	}
	const bool bSenderTeam = Target->PlayerState_0x118->GetTeamKey_Slot0x28() != 0;
	const bool bLocalTeam = TeamState_0x590->GetTeamKey_Slot0x28() != 0;
	if (bSenderTeam == bLocalTeam)
	{
		FText ChatTextCopy = ChatText;
		SendPresetTeamChat_0x142a3c240(Target, SenderID, ChatTextCopy);
	}
}
