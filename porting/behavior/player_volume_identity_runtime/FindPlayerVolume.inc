// CONFIDENCE: HIGH
QUICK_SCOPE_CYCLE_COUNTER(STAT_Find_Volume);
if (!IsValid(PlayerState))
{
    return nullptr;
}
return NativeFindPlayerVolumeInActiveUsers(PlayerState);
