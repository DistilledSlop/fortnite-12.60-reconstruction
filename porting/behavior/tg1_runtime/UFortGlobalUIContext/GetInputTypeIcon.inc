// Original body 0x144ccce30 (thunk 0x145247d80). Evidence: no receiver state; UFortRarityData common game data getter then tail-jump to NativeFindByteKeyEntry141e60d60 with the input type byte.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortRarityData) > 0, "UFortRarityData must be complete for the common data getter.");
static_assert(sizeof(UFortGameData) > 0, "UFortGameData must be complete for the map owner conversion.");
UFortGameData* GameDataObj = UFortRarityData::NativeGetCommonGameData141e5b2a0();
return reinterpret_cast<UTexture2D*>(UFortGlobalUIContext::NativeFindByteKeyEntry141e60d60(GameDataObj, static_cast<uint8>(InputType)));
