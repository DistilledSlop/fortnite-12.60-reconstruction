// CONFIDENCE: high; complete captured native branches and typed dependencies.
// Native 0x1429c3b90; exec 0x1429c3b90.
const int32 RoundWinner = GetRoundWinner(CurrentRound);
const int32 PlayerBucket = GetBucketForPlayerId(PlayerId);
return RoundWinner == PlayerBucket;
