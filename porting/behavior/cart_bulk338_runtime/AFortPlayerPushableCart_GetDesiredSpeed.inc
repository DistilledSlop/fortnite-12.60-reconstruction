// CONFIDENCE: high; native wrapper and body captured.
{
    if (!bIsPushable || TotalActiveDefenders != 0 || TotalActivePushers == 0)
        return 0.0f;
    const float PusherScale = PushSpeedPercentPerPusher.GetValueAtLevel(static_cast<float>(TotalActivePushers), nullptr);
    const float Speed = BasePushSpeed.GetValueAtLevel(0.0f, nullptr);
    return Speed * PusherScale;
}
