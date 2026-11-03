// Integration proposal; original names are unknown. These two callbacks pass
// the boolean byte by value in the Windows x64 integer argument registers.
DECLARE_MULTICAST_DELEGATE_OneParam(FPlayerPreserveSquadChangedNative, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerResurrectionChipChangedNative, AFortPlayerStateAthena*, bool);
// AFortPlayerStateAthena fields:
// FPlayerPreserveSquadChangedNative PreserveSquadChangedNative;       // +0x13e8
// FPlayerResurrectionChipChangedNative ResurrectionChipChangedNative; // +0x1340
// Root integrates +0xda0 as TMulticastDelegate<void, ETeamMemberState> and
// +0xd70 as FTimerHandle; original C++ type spelling remains a source choice.
