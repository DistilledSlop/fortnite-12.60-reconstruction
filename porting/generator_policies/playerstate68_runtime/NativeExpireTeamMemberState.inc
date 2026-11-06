// CONFIDENCE: HIGH
// Original 0x141a58b00: authority clears only the replicated state.
if (GetLocalRole() == ROLE_Authority)
{
    ReplicatedTeamMemberState = ETeamMemberState::None;
}
else if (TeamMemberState != ETeamMemberState::None)
{
    TeamMemberState = ETeamMemberState::None;
    TeamMemberStateChangedNative.Broadcast(TeamMemberState);
}
