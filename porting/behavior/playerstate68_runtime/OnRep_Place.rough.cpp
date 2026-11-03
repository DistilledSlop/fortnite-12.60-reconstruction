// Incomplete source draft: the exact meanings/types of the native helper effects remain unresolved.
// Names below are explicit external integration contracts, not claims about original names.
extern uint8 PlaceStatus_141a3c6f0(AFortPlayerStateAthena*);
extern void BroadcastPlaceStatus_140e3aa70(AFortPlayerStateAthena*, uint8); // multicast +0x1358
extern void* PlaceOwnerAtE0(AFortPlayerStateAthena*);
extern bool IsType_142a6ee20(void*); // reproduces original class hierarchy check
extern void NotifyOwner_1423defc0_140684de0(void*);
extern void* WorldService_1426402a0(UWorld*);
extern bool WorldServiceEnabled_14264e810(void*);
extern uint32 WorldValue_142640280(UWorld*);
extern void* FindPlayerEntry_14263f280(void*, AFortPlayerStateAthena*); // uses unique id +0x248
extern void UpdateEntryPlace_142655b70(void*, uint32, int32);
void AFortPlayerStateAthena::OnRep_Place()
{
    PlaceChanged.Broadcast(this, Place);
    BroadcastPlaceStatus_140e3aa70(this, PlaceStatus_141a3c6f0(this));
    if (void* Owner = PlaceOwnerAtE0(this))
        if (IsType_142a6ee20(Owner)) NotifyOwner_1423defc0_140684de0(Owner);
    if (UWorld* World = GetWorld())
        if (void* Service = WorldService_1426402a0(World))
            if (WorldServiceEnabled_14264e810(Service)) {
                const uint32 Value = WorldValue_142640280(World);
                void* Entry = FindPlayerEntry_14263f280(Service, this);
                UpdateEntryPlace_142655b70(Entry, Value, Place);
            }
}
