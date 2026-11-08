// Helper 0x144c1b2a0 (UAthenaRemotePlayerViewData static, 2 callers: 0x144c20070 with delegate +0x78 OnLocationChanged,
// 0x144c201f0 with delegate +0x58 OnPlayerNameChanged; both tail-jump or call with RDX = PlayerData, R8 = FString*).
// The asm copies the FString into a temporary (TArray Reserve 0x140690ac0, memcpy 0x1458092ed, free 0x142bee710),
// then broadcasts the FStringDelegate with the pair (PlayerData, copy). The copy and its release are emitted by C++.
// CONFIDENCE: HIGH
Delegate->Broadcast(PlayerData, StringParam);
