// Translation of HideToast (body 0x144e0c9c0) and wrapper 0x142292260.
// NOT COMPILABLE and NOT INTEGRATED: depends on UNKNOWN layouts and the placeholders in declarations.inc.

// 0x142292260 wrapper. Walks Outer hops from TeamMember, then calls the vtable slot 0x268 for the PlayerId.
// VERIFIED (asm 0x142292273-0x14229227f): the 0x10 test reads TeamMember+0x8 (not Hop1); bit 0x10 = RF_ClassDefaultObject.
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
    uint8* Hop2 = NativeOuterOrNull140f930b0(Hop1);       // VERIFIED 0x140f930b0: no null guard, reads [rcx+0x8] (Hop1 may be 0)
    uint8* GameInstance = *(uint8**)(Hop2 + 0x20);        // Hop2->Outer

    NativeToastIdOut142292260 IdOut;                      // VERIFIED: buffer at wrapper rsp+0x20 (lea at 0x142292284), passed in rdx; layout UNKNOWN
    // Virtual slot 0x268 (index 77) on TeamMember (call *0x268(%rax) at 0x142292293). Returns rax = buffer address per pseudo-C; the slot's owner class UNKNOWN.
    uint64 IdRef = ((uint64 (*)(void*, void*))((*(void***)TeamMember)[77]))(TeamMember, &IdOut);
    UFortRegisteredPlayerInfo* Player = NativeFindToastRegisteredPlayer141e62c80(GameInstance, (const void*)IdRef);
    *(void**)&IdOut = NativeVtable145a95888;              // VERIFIED store mov %rax,0x20(%rsp) at 0x1422922b3, after both calls

    // VERIFIED (NOTES.md round 2): lStack_20 = IdOut+0x18 (rsp+0x38, read 0x1422922a4) is written 0 by the 0x268 callee
    // (0x142292235) on all paths except path C (0x1422921bd); plStack_28 = IdOut+0x10 (rsp+0x30, read 0x1422922c2) is the
    // callee's refcounted pointer (obj+0x240 or obj+0x258, incl at 0x1422921b6) or 0. Path C UNKNOWN.
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
    // VERIFIED (asm 0x144e0c9c0-0x144e0c9d4, 0x144e0c9e1): NativeTeamMember is the weak slot at this+0x290.
    if (NativeWeakPtrIsValid142ee7880(&NativeTeamMember))
    {
        uint8* TeamMember = NativeWeakPtrGet142ee5dd0(&NativeTeamMember);
        NativeMulticastRemoveAll140690620(TeamMember + 0x78, this);         // NativeMulticast78; VERIFIED args rcx=TeamMember+0x78, rdx=this (0x144e0c9f2-0x144e0c9f9)

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
        // VERIFIED: movl $0xffffffff,0x290(%rbx) (0x144e0ca64) and movl $0x0,0x294(%rbx) (0x144e0ca6e).
        // Fields match TWeakObjectPtr { int32 ObjectIndex; int32 ObjectSerialNumber; } (WeakObjectPtr.h:235-236).
        *(int32*)((uint8*)this + 0x290) = -1;
        *(int32*)((uint8*)this + 0x294) = 0;
    }
    // VERIFIED: runs on both paths (je 0x144e0ca78 at 0x144e0c9db). lea 0x258(%rbx) = &OnToastFinished (0x144e0ca78).
    // Second argument: xor %edx,%edx (0x144e0ca7f) = 0 at the call site; its meaning is UNKNOWN.
    NativeDynamicMulticastBroadcast1406a9dc0(&OnToastFinished, 0);
}

