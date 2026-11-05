// CONFIDENCE: high; complete captured native branches and typed dependencies.
// Native 0x1429c3cb0; exec 0x1429c3cb0.
const int32 RoundWinner = GetRoundWinner(CurrentRound);
const int32 PlayerBucket = GetBucketForPlayerState(PlayerState);
return RoundWinner == PlayerBucket;
