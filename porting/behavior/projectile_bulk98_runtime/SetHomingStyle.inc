// Original exec: 0x142aa6bf0; native body: 0x141d15d70.
// CONFIDENCE: high
HomingData.HomingStyle = NewHomingStyle;
HomingData.ResetTurnSpeedTimer = ResetHomingTurnSpeedTimer;
if (ResetHomingTurnSpeedTimer)
{
    HomingTravelTime = 0.0f;
}
