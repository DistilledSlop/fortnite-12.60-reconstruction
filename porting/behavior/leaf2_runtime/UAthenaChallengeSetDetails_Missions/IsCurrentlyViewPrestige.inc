// Behaviour snippet: UAthenaChallengeSetDetails_Missions::IsCurrentlyViewPrestige (body only; spliced into the generated member function).
// Original body address: 0x144b7ef60 (thunk 0x14518cbd0)
// Offset evidence: Pointer at +0x640 = ChallengeDetails_Rewards (reflection 1600); dword at pointee +0x280 = CurrentBundleLevelBeingViewed (reflection 640, Int). No null check in asm.
// CONFIDENCE: MEDIUM

UAthenaChallengeDetailRewardTrack* RewardTrack = ChallengeDetails_Rewards;
return RewardTrack->CurrentBundleLevelBeingViewed != 0;
