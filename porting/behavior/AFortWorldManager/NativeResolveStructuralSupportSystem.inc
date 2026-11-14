// CONFIDENCE: HIGH
// Complete 0x141c77490. Requires Engine/Engine.h, Engine/World.h,
// AFortGameStateZone.h and UBuildingStructuralSupportSystem.h.
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
AFortGameStateZone* State = World ? Cast<AFortGameStateZone>(World->GetGameState()) : nullptr;
return State ? State->StructuralSupportSystem : nullptr;
