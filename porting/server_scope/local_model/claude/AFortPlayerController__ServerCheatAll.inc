// CONFIDENCE: LOW - validate and dispatch are read; the candidate filter and actor iteration (0x140894360 iterator, 0x143f76730, 0x1413064f0, 0x14480eb00, 0x14468e190) are only partly decoded, and the per-actor cheat virtual (slot 0xc20 on the candidate) and the event dispatch (0x1447ba4b0, 0x14469a960) are not ported.
// Original address: 0x142a4a590 (RPC exec thunk, Msg string). Validate is virtual slot 0xe10 -> Stub_ReturnTrue (0x1406801d0).
// Implementation is virtual slot 0xe18 -> 0x1423aa300 (247 lines). Its flow: gate on this->+0x330, find the anchor actor (first FortPlayerController from the iterator), then sweep candidate actors and run the cheat on each match.
if (!ServerCheatAll_Validate(Msg))
{
	RPC_ValidateFailed(TEXT("ServerCheatAll_Validate"));
	return;
}
if (CheatTarget_0x330 == nullptr)
{
	return;
}
AActor* Anchor = FirstFortPlayerControllerInWorld_0x140894360(GetWorld());
// Candidate sweep over the world actor list (two source arrays, starting after the iterator position).
// Filter: skip pending-kill objects (object flag bit 29 in the object table); skip when the stub predicate (Stub_ReturnFalse) is true;
// when the strict flag (bit 2) is set, also require the controller flag checks (+0x1f5 bit 0x40 / +500 bit 0x20) and the pawn ownership test.
for (AActor* Candidate : WorldActorCandidates(GetWorld(), Anchor))
{
	if (Candidate == nullptr || Candidate->IsPendingKill())
	{
		continue;
	}
	if (ActorOwnerInGroup_0x1413064f0(Candidate) != Anchor)
	{
		continue;
	}
	// Match: run the cheat on this candidate (virtual slot 0xc20), then dispatch the completion event with the message.
	Candidate->ExecuteCheat_Slot0xc20(Msg);
	CompleteCheatDispatch_0x1447ba4b0(Candidate, Msg);
}
// Cleanup of the iterator state (0x14469a960 on the anchor list, then release of the temporary candidate arrays) follows the sweep.
ReleaseCheatIteratorState_0x14469a960(Anchor);
