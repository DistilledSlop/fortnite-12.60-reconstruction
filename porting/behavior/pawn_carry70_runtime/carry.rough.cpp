// UNCOUNTED, not a build input. Neutral offsets/calls identify original native
// effects and unresolved ABI. No adapter below supplies fallback behavior.
namespace pawn_carry70 {
struct Object;
struct Pawn;
struct Globals;
struct Tags;
extern Object*& ObjectAt(void*, unsigned);
extern Pawn*& PawnAt(void*, unsigned);
extern uint8& ByteAt(void*, unsigned);
extern uint16& ShortAt(void*, unsigned);
extern uint64& QwordAt(void*, unsigned);
extern uint32& DwordAt(void*, unsigned);
extern void* Address(void*, unsigned);
extern bool V6b0(Pawn*);
extern void V5e8(Pawn*);
extern Object* VInterface10(void*);
extern uint8 VInterface28(void*);
extern void V800(Object*, int, int);
extern void V618(Object*, int);
extern void V810(Object*, void* Tag, int);
extern bool V11c8(Pawn*, Object* Controller);
extern bool IsA(Object*, void*);
extern void* Class_142a3d9f0();
extern void* Class_142a85850();
extern void* Class_142a62c80();
extern Globals* Globals_141ed1800();
extern bool UpdateTagCount_140744d30(void* Map, void* Tag, int Delta);
extern void HoisterChanged_142336990(Pawn*, Pawn* PreviousHoister, uint8 Action);
extern void Broadcast_1406a9dc0(void*, const void*);
extern void TagsInitialize(Tags&);
extern void TagsAdd_143e8b3b0(Tags&, void*);
extern void RemoveTags_1406fd020(Object*, Tags&, int, int);
extern void TagsDestroyArraysInOrder(Tags&); // +0x10 array, then +0 array
extern FVector GlobalVector_148218630();
extern FVector ReadVector(void*);
extern void WriteVector(void*, const FVector&);
extern void ComponentCall_1440d4a00_UnresolvedArgs();
extern void MarkGlobalObjectFlag29_FromObjectIndex(Object*);
// Must reproduce native chunk/index lookup, including its unchecked/null path;
// this is not an inferred safe UObject MarkPendingKill replacement.
extern void AttachmentRPC_142a775e0(Pawn*, const void* Mesh16Bytes);
extern bool AttachmentWarningLevelAbove2();
extern void LogAttachmentWarning_WithNativeNameTemporary(Pawn*);

// exec142a7e770. No capsule null guard; interface getter re-evaluated for each
// globals lookup. Tag order is 520,530,528, not monotonically increasing.
void RestoreDBNOCollisionAfterDrop(Pawn* Self)
{
    if ((ByteAt(Self, 0x58) & 0x40) || PawnAt(Self, 0x1f48)) return;
    V800(ObjectAt(Self, 0x288), 2, 2);
    V800(ObjectAt(Self, 0x288), 0, 2);
    Object* Collision = ObjectAt(Self, U_DBNOInteractCollisionComponentOffset);
    if (Collision) V618(Collision, 1);
    void* Interface = Address(Self, 0x4c8);
    if (VInterface10(Interface)) {
        for (unsigned TagOffset : {0x520u, 0x530u, 0x528u}) {
            Globals* Global = Globals_141ed1800();
            Object* Ability = VInterface10(Interface);
            void* Tag = Address(Global, TagOffset);
            if (UpdateTagCount_140744d30(Address(Ability, 0x1040), Tag, -1))
                V810(Ability, Tag, 0);
        }
    }
}

// exec142a7d6d0. Deliberate rereads after callback; no hoistee cached across it.
void LocalThrowCarriedPlayer(Pawn* Self, Tags& OpaqueLocalTagStorage)
{
    Pawn* Hoistee = PawnAt(Self, 0x1f58);
    if (!Hoistee || PawnAt(Hoistee, 0x1f48) != Self) return;
    PawnAt(Hoistee, 0x1f48) = nullptr;
    ByteAt(PawnAt(Self, 0x1f58), 0x1f50) = 3;
    HoisterChanged_142336990(PawnAt(Self, 0x1f58), Self, 3);
    Object* Controller = ObjectAt(Self, 0x250);
    PawnAt(Self, 0x1f58) = nullptr;
    if (Controller && IsA(Controller, Class_142a62c80())) {
        uint8 Payload = PawnAt(Self, 0x1f58) != nullptr;
        Broadcast_1406a9dc0(Address(Controller, 0x2470), &Payload);
    }
    if (ByteAt(Self, 0xf0) == 3 && VInterface10(Address(Self, 0x4c8))) {
        Globals* Global = Globals_141ed1800();
        TagsInitialize(OpaqueLocalTagStorage);
        TagsAdd_143e8b3b0(OpaqueLocalTagStorage, Address(Global, 0x15b8));
        Global = Globals_141ed1800();
        TagsAdd_143e8b3b0(OpaqueLocalTagStorage, Address(Global, 0x15c0));
        Object* Ability = VInterface10(Address(Self, 0x4c8));
        RemoveTags_1406fd020(Ability, OpaqueLocalTagStorage, 0, 0);
        TagsDestroyArraysInOrder(OpaqueLocalTagStorage);
    }
}

// native14234a360. Statistics use modulo-16-bit increment. No extra eligibility
// check invented beyond target virtual11c8 with the hoister controller.
void ServerHoistDBNOPlayer_Implementation(Pawn* Self, Pawn* Target)
{
    if (!Target || !Self) return;
    Object* Controller = ObjectAt(Self, 0x250);
    if (!Controller || !IsA(Controller, Class_142a3d9f0()) ||
        !V11c8(Target, Controller)) return;
    PawnAt(Target, 0x1f48) = Self;
    ByteAt(Target, 0x1f50) = 0;
    PawnAt(Self, 0x1f58) = Target;
    Object* TargetState = ObjectAt(Target, 0x238);
    if (TargetState && IsA(TargetState, Class_142a85850())) {
        uint8 FirstTeam = VInterface28(Address(Self, 0x4a8));
        uint8 SecondTeam = VInterface28(Address(Target, 0x4a8));
        uint16& Counter = ShortAt(TargetState, FirstTeam == SecondTeam ? 0x952 : 0x954);
        Counter = uint16(Counter + 1u);
        Object* Root = ObjectAt(Target, 0x130);
        FVector Location = Root ? ReadVector(Address(Root, 0x1d0))
                                : GlobalVector_148218630();
        WriteVector(Address(TargetState, 0x964), Location);
    }
    HoisterChanged_142336990(Target, nullptr, 0);
    V5e8(Target);
}

// exec142a7e4c0. Saved role byte is reused after callbacks. Attachment mesh is
// a 16-byte native value; clearing it does not imply a reconstructed pointer.
void RemoveAttachment(Pawn* Self)
{
    void* Mesh = Address(Self, U_AttachmentMeshOffset);
    if (!QwordAt(Mesh, 0)) return;
    bool Local = V6b0(Self);
    uint8 Role = ByteAt(Self, 0xf0);
    if (!Local && Role != 3) {
        if (AttachmentWarningLevelAbove2())
            LogAttachmentWarning_WithNativeNameTemporary(Self);
        return;
    }
    QwordAt(Mesh, 0) = 0;
    QwordAt(Mesh, 8) = 0;
    Object*& Component = ObjectAt(Self, U_AttachmentMeshComponentOffset);
    if (Component) {
        ComponentCall_1440d4a00_UnresolvedArgs();
        MarkGlobalObjectFlag29_FromObjectIndex(Component);
        Component = nullptr;
    }
    if (Role != 3) {
        // Native copies mesh after component destruction; reread all four
        // words, rather than assuming callbacks have left it cleared.
        uint32 Copy[4] = {DwordAt(Mesh, 0), DwordAt(Mesh, 4),
                          DwordAt(Mesh, 8), DwordAt(Mesh, 12)};
        AttachmentRPC_142a775e0(Self, Copy);
    }
}
} // namespace pawn_carry70
