// CONFIDENCE: MEDIUM - world resolution and class check read in full; the flag at +0x398 bit 0x4 is a proposed name.
// Original addresses: exec 0x142725f50; body 0x141737a80
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
UNavigationSystemV1* NavSys = World ? Cast<UNavigationSystemV1>(World->NavigationSystem) : nullptr;
return NavSys != nullptr && (NavSys->NavigationFlags398 & 0x4) != 0;
