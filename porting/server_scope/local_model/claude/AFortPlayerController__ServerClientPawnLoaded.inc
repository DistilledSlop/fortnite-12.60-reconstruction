// CONFIDENCE: HIGH. Original exec 0x142a4a710 (RPC thunk, one bool parameter bIsPawnLoaded).
// Validate is virtual slot 0x1330 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0x1338 -> 0x1423aa960 (25 lines), inlined below.
// Flag word at this+0x658 (uint32): bit 0x40 holds the pawn-loaded state (member name not recovered).
const uint32 PawnLoadedBit = (bIsPawnLoaded ? 1u : 0u) << 6;
PawnLoadedFlags_0x658 = PawnLoadedBit | (PawnLoadedFlags_0x658 & ~0x40u);
if ((PawnLoadedBit & 0x40u) != 0)
{
	UWorld* World = GetWorld();
	AGameStateBase* GameState = World ? World->GameState : nullptr;
	if (GameState != nullptr && GameState->IsA(AFortGameStateZone::StaticClass()))
	{
		GameState->OnPlayerPawnLoaded_Slot0x9c8(this);
	}
}
