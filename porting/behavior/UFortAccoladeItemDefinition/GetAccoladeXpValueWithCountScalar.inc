// CONFIDENCE: high
// Native body at 0x142008040 evaluates XpRewardAmount at AccoladeLevel and XpRewardScalarByCount at Count.
const float BaseAmount = XpRewardAmount.GetValueAtLevel(AccoladeLevel, nullptr);
const float CountScalar = XpRewardScalarByCount.GetValueAtLevel(static_cast<float>(Count), nullptr);
return static_cast<int32>(CountScalar * static_cast<float>(static_cast<int32>(BaseAmount)));
