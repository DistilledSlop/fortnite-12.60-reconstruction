// CONFIDENCE: HIGH
// 0x141fc0b70. The game-mode provider gates access before resolving/tracking.
if (!UFortGameInstance::NativeIsGameMode141ed8de0(nullptr, 0))
    return nullptr;
return UGameDataSTW::NativeGetGameDataSTW()->ResolveTimeOfDayDataNative();
