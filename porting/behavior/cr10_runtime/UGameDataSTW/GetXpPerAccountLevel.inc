// Original body address: 0x1424e9f80 (thunk 0x142b63d80)
// Offset evidence: CALL 0x141df62a0 (UFortAssetManager::NativeGetFortAssetManager) then CALL 0x141df78c0 (NativeGetGameDataSTW); MOV EAX,dword [RAX+0xf0] reads UGameDataSTW +0xf0, which lookup_offset.py reports as reflected XpPerAccountLevel (Int).
// CONFIDENCE: HIGH
return UFortAssetManager::NativeGetFortAssetManager().NativeGetGameDataSTW()->XpPerAccountLevel;
