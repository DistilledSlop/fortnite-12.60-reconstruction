// Behaviour snippet: UFortMissionActivationInfo::CanRequestVote (body only).
// Original body address: 0x144f54e60 (thunk 0x145289420). this_delta_total 0.
// Offset evidence: [RCX+0xa8] = reflected member GameStateZone (AFortGameStateZone*, object offset 0xa8, lookup_offset.py). Null returns false (XOR AL,AL); otherwise the body returns !GameStateZone->IsGameplayVoteInProgress(VoteType) (callee 0x141ea2470 = AFortGameStateZone::IsGameplayVoteInProgress, declared const in AFortGameStateZone.h; VoteType stays in RDX).
// CONFIDENCE: HIGH
if (GameStateZone == nullptr)
{
    return false;
}
return !GameStateZone->IsGameplayVoteInProgress(VoteType);
