// Original body 0x1424e8c90 (thunk 0x142b63d50). Offset evidence: NativeGetGameDataSTW on the asset manager (0x141df78c0) returns UGameDataSTW. The MOVSS from +0xfc is GroupBoost_XpBonusPercent (reflected Float, exact lookup).
// CONFIDENCE: HIGH
return UFortAssetManager::NativeGetFortAssetManager().NativeGetGameDataSTW()->GroupBoost_XpBonusPercent;
