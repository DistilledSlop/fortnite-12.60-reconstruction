// Original body 0x14235d480 (thunk 0x14286b9a0). Offset evidence: PC+0x330 = APlayerController::CheatManager (reflected Object). Class-tree test against UFortCheatManager::StaticClass() is IsA (Cast). CheatManager+0x88 dword bit 0x1000 = byte 0x89 mask 0x10 = UFortCheatManager::bEnableContextMenus (reflected uint8 bitfield, offset 137 mask 16).
// CONFIDENCE: MEDIUM
if (!PC)
{
	return false;
}
UFortCheatManager* FortCheatManager = Cast<UFortCheatManager>(PC->CheatManager);
if (!FortCheatManager)
{
	return false;
}
return FortCheatManager->bEnableContextMenus != 0;
