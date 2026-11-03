// CONFIDENCE: HIGH
if (ReplicatedTeamMemberState == ETeamMemberState::None)
{
    return;
}
TeamMemberState = ReplicatedTeamMemberState;
TeamMemberStateRepTime = GetWorld()->GetTimeSeconds();
FTimerManager& Timers = GetWorld()->GetTimerManager();
Timers.ClearTimer(TeamMemberStateTimer);
TeamMemberStateTimer.Invalidate();
Timers.SetTimer(TeamMemberStateTimer,
    FTimerDelegate::CreateUObject(this, &AFortPlayerStateAthena::NativeExpireTeamMemberState),
    3.0f, false);
TeamMemberStateChangedNative.Broadcast(TeamMemberState);
