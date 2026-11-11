// CONFIDENCE: MEDIUM
// Native 0x14296c540 -> 0x141ef0ee0: every output is reset first. The seasonal event manager of the fort game instance is
// queried by EventName. When the event is found, TimeSinceBegin and TimeUntilEnd are filled, TimeSpanRatio is
// since / (since + until) and bIsEventActive is set. A missing game instance or manager leaves the outputs at zero.
bIsEventActive = false;
TimeSinceBegin = FTimespan::Zero();
TimeUntilEnd = FTimespan::Zero();
TimeSpanRatio = 0.0f;
const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
const UFortGameInstance* GameInstance = World ? Cast<UFortGameInstance>(World->GetGameInstance()) : nullptr;
if (!GameInstance || !GameInstance->SeasonalEventManager)
{
    return;
}
UFortSeasonalEventManager* EventManager = GameInstance->SeasonalEventManager;
if (EventManager->GetEventTiming(FName(*EventName), TimeSinceBegin, TimeUntilEnd))
{
    const int64 TotalTicks = TimeSinceBegin.GetTicks() + TimeUntilEnd.GetTicks();
    TimeSpanRatio = TotalTicks != 0 ? static_cast<float>(static_cast<double>(TimeSinceBegin.GetTicks()) / static_cast<double>(TotalTicks)) : 0.0f;
    bIsEventActive = true;
}
