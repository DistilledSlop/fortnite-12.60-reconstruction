// CONFIDENCE: MEDIUM - dispatch and validate path fully read; the announcement removal helper 0x14187f4e0 (85 lines) is read but its callees (0x141872540, 0x14186e680, 0x1407ef240, 0x14187f780, 0x14072c6c0, 0x14186f270) are not ported.
// Original address: 0x142a4a060 (RPC exec thunk, AnnouncementID struct). Validate is virtual slot 0x1288 -> Stub_ReturnTrue (0x1406801d0).
// Implementation is virtual slot 0x1290 -> 0x1423aa2a0, which forwards to the announcement subsystem found from the world.
if (!ServerAnnouncementStoppedOnClient_Validate(AnnouncementID))
{
	RPC_ValidateFailed(TEXT("ServerAnnouncementStoppedOnClient_Validate"));
	return;
}
UObject* World = GetWorld();
UObject* Subsystem = GetAnnouncementSubsystem_0x14186f5b0(World);
if (Subsystem != nullptr)
{
	AnnouncementStopped_0x14187f4e0(Subsystem, AnnouncementID, this);
}
