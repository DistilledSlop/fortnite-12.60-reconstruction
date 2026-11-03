// UFortPlayerPowerRatingToast::HideToast, body 0x144e0c9c0 (204 bytes), verified against the runtime dump.
//
// NOT COMPILABLE AND NOT INTEGRATED. The four UNRECOVERED_* identifiers below mark native items
// whose type or name has not been recovered. Do not replace them with substitute APIs.
//   UNRECOVERED_TEAM_MEMBER_TOAST_MULTICAST : UFortTeamMember native multicast at +0x78.
//   UNRECOVERED_GAME_INSTANCE_FROM_TEAM      : helper 0x140f930b0 applied to the team member (+0x20 read).
//   UNRECOVERED_FIND_REGISTERED_PLAYER       : wrapper 0x142292260. It takes the team identity via virtual
//                                              +0x268 (0x142292160, ResolveToastTeamMemberId) and calls the
//                                              hand-written FindToastRegisteredPlayer (0x141e62c80).
//   UNRECOVERED_REGISTERED_PLAYER_MULTICAST  : UFortRegisteredPlayerInfo native multicast at +0x250.
//   UNRECOVERED_PROFILE_MULTICAST            : UMcpProfile native multicast at +0xe8, used for both profiles.
// Addresses: IsValid 0x142ee7880, Get 0x142ee5dd0, MulticastRemoveAll 0x140690620,
// OnToastFinished broadcast tail 0x1406a9dc0 (the dynamic multicast at +0x258 is broadcast with no args).

void UFortPlayerPowerRatingToast::HideToast()
{
    if (NativeTeamMember.IsValid())
    {
        UFortTeamMemberCampaign* TeamMember = NativeTeamMember.Get();
        UNRECOVERED_TEAM_MEMBER_TOAST_MULTICAST.RemoveAll(this);

        TeamMember = NativeTeamMember.Get();
        if (UFortRegisteredPlayerInfo* Player = UNRECOVERED_FIND_REGISTERED_PLAYER(UNRECOVERED_GAME_INSTANCE_FROM_TEAM(TeamMember), TeamMember))
        {
            UNRECOVERED_REGISTERED_PLAYER_MULTICAST.RemoveAll(this);

            if (Player->CommonPublicProfile)
            {
                UNRECOVERED_PROFILE_MULTICAST(Player->CommonPublicProfile).RemoveAll(this);
            }
            if (Player->AccountProfile)
            {
                UNRECOVERED_PROFILE_MULTICAST(Player->AccountProfile).RemoveAll(this);
            }
        }

        // Reset the weak reference (ObjectIndex = -1, SerialNumber = 0). This runs inside the valid branch.
        NativeTeamMember = nullptr;
    }

    OnToastFinished.Broadcast();
}
