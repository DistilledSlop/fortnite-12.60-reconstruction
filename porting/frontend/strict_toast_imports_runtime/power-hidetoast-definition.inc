// UFortPlayerPowerRatingToast::HideToast, body 0x144e0c9c0 (204 bytes), verified against the runtime dump.
//
// NOT COMPILABLE AND NOT INTEGRATED. The UNRECOVERED_* identifiers below mark native items whose
// type or name has not been recovered. Do not replace them with substitute APIs.
//
// Call-chain correction (from disassembly): HideToast does NOT call a separate game-instance helper.
// It passes the team member straight to the wrapper 0x142292260 (call at body +0x4d, rcx = TeamMember).
// The helper 0x140f930b0 is called inside that wrapper. Recovered wrapper behavior:
//   1. Reads TeamMember->Outer (+0x20), unless TeamMember's ObjectFlags (+0x8) bit 0x10 is set (then null).
//   2. Calls 0x140f930b0 on that value. It returns 0 when the flag bit is set, otherwise the value's Outer (+0x20).
//   3. Reads the Outer (+0x20) of that result again. This third value is passed as the game-instance argument
//      to FindToastRegisteredPlayer (0x141e62c80, see ToastRegisteredPlayerLookup.h).
//   4. The PlayerId comes from virtual +0x268 on TeamMember (0x142292160, ResolveToastTeamMemberId).
// The type of the third outer object is not confirmed as UFortGameInstance from the binary alone.
//
// Unresolved (marked UNRECOVERED_*):
//   UNRECOVERED_TEAM_MEMBER_TOAST_MULTICAST : native multicast at UFortTeamMember +0x78 (body uses RemoveAll(this) only).
//                                             Reflection record 0x14652f830 has one reflected property (SocialUser, +0x28),
//                                             so +0x78 is a non-UPROPERTY native member. Type and name unknown.
//   UNRECOVERED_FIND_REGISTERED_PLAYER      : wrapper 0x142292260, signature recovered as
//                                             UFortRegisteredPlayerInfo* (UFortTeamMember* TeamMember). Name unknown.
//   UNRECOVERED_REGISTERED_PLAYER_MULTICAST : native multicast at UFortRegisteredPlayerInfo +0x250. Reflected properties
//                                             jump from +0x1d8 to +0x2f8, so +0x250 is a native member. Type and name unknown.
//   UNRECOVERED_PROFILE_MULTICAST           : native multicast at UMcpProfile +0xe8, used for both profiles. UMcpProfile
//                                             reflected properties end at +0xd0, so +0xe8 is native. Type and name unknown.
//
// Recovered and reflection-confirmed: Player+0xd8 = CommonPublicProfile, Player+0xe8 = AccountProfile
// (reflection record 0x146502df0). Also OnToastFinished is reflected at +0x258 (record 0x146f9f600).
//
// Addresses: IsValid 0x142ee7880, Get 0x142ee5dd0, MulticastRemoveAll 0x140690620,
// OnToastFinished broadcast tail 0x1406a9dc0 (the dynamic multicast at +0x258 is broadcast with no args).

void UFortPlayerPowerRatingToast::HideToast()
{
    if (NativeTeamMember.IsValid())
    {
        UFortTeamMemberCampaign* TeamMember = NativeTeamMember.Get();
        UNRECOVERED_TEAM_MEMBER_TOAST_MULTICAST.RemoveAll(this);

        TeamMember = NativeTeamMember.Get();
        if (UFortRegisteredPlayerInfo* Player = UNRECOVERED_FIND_REGISTERED_PLAYER(TeamMember))
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
