// CONFIDENCE: high; captured native flow and typed dependency behavior.
{
    const float Remaining = GetMissionTimerTimeRemaining();
    return TimerData.OriginalTimePeriod > 0.0f ? Remaining / TimerData.OriginalTimePeriod : 0.0f;
}
