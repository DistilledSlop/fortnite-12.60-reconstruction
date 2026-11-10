// CONFIDENCE: HIGH
// Original target 0x1419f1380.
if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerStateAthena::StaticClass()))
{
	PlayerState->SetReadyToRespawnFlag(true);	// FUN_141a59620 writes a bool at +0xd31
}
