// CONFIDENCE: high. Native target 0x1450cce80.
if (RewardStatus != Status)
{
    RewardStatus = Status;
    OnRewardStatusChanged(Status);
}
