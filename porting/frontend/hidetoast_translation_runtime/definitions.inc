// Translation of HideToast (body 0x144e0c9c0) and wrapper 0x142292260.
// NOT COMPILABLE and NOT INTEGRATED: depends on UNKNOWN layouts and the placeholders in declarations.inc.

// 0x142292260 wrapper. Walks Outer hops from TeamMember, then calls the vtable slot 0x268 for the PlayerId.
UFortRegisteredPlayerInfo* NativeToastFindRegisteredPlayer142292260(UFortTeamMemberCampaign* TeamMember)
{
    uint8* Hop1;
    if ((*(uint32*)((uint8*)TeamMember + 0x8) >> 4 & 1) == 0)
    {
        Hop1 = *(uint8**)((uint8*)TeamMember + 0x20);     // TeamMember->Outer
    }
    else
    {
        Hop1 = nullptr;
    }
    uint8* Hop2 = NativeOuterOrNull140f930b0(Hop1);       // no null guard in the pseudo-C (see NOTES.md)
    uint8* GameInstance = *(uint8**)(Hop2 + 0x20);        // Hop2->Outer

    NativeToastIdOut142292260 IdOut;                      // hidden return buffer (rdx in the disassembly)
    // Virtual slot 0x268 (index 77) on TeamMember. Returns the buffer address (pseudo-C undefined8 uVar4).
    uint64 IdRef = ((uint64 (*)(void*, void*))((*(void***)TeamMember)[77]))(TeamMember, &IdOut);
    UFortRegisteredPlayerInfo* Player = NativeFindToastRegisteredPlayer141e62c80(GameInstance, (const void*)IdRef);
    *(void**)&IdOut = NativeVtable145a95888;              // appuStack_38[0] = vtable (store after the calls)

    if (UNKNOWN_lStack_20 != nullptr)
    {
        NativeFreeData142bee710(UNKNOWN_lStack_20);
    }
    NativeCtrlRelease142292260(UNKNOWN_plStack_28);
    return Player;
}

// 0x144e0c9c0 HideToast body. Pseudo-C: IsValid, RemoveAll chain, reset weak slot, broadcast.
void UFortPlayerPowerRatingToast::HideToast()
{
    if (NativeWeakPtrIsValid142ee7880(&NativeTeamMember))
    {
        uint8* TeamMember = NativeWeakPtrGet142ee5dd0(&NativeTeamMember);
        NativeMulticastRemoveAll140690620(TeamMember + 0x78, this);         // NativeMulticast78

        uint8* TeamMember2 = NativeWeakPtrGet142ee5dd0(&NativeTeamMember);
        UFortRegisteredPlayerInfo* Player = NativeToastFindRegisteredPlayer142292260((UFortTeamMemberCampaign*)TeamMember2);
        if (Player != nullptr)
        {
            NativeMulticastRemoveAll140690620((uint8*)Player + 0x250, this);  // NativeMulticast250
            if (*(void**)((uint8*)Player + 0xd8) != nullptr)                    // CommonPublicProfile
            {
                NativeMulticastRemoveAll140690620(*(uint8**)((uint8*)Player + 0xd8) + 0xe8, this); // NativeMulticastE8
            }
            if (*(void**)((uint8*)Player + 0xe8) != nullptr)                    // AccountProfile
            {
                NativeMulticastRemoveAll140690620(*(uint8**)((uint8*)Player + 0xe8) + 0xe8, this); // NativeMulticastE8
            }
        }
        // Weak reset in place (ObjectIndex -1, SerialNumber 0), as in the pseudo-C.
        *(int32*)((uint8*)this + 0x290) = -1;
        *(int32*)((uint8*)this + 0x294) = 0;
    }
    // Runs unconditionally (pseudo-C tail).
    NativeDynamicMulticastBroadcast1406a9dc0(&OnToastFinished, 0);
}

