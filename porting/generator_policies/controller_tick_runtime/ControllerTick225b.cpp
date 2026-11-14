#include "ControllerTick225b.h"
#include "AFortPlayerController.h"
#include "AFortPlayerCameraZone.h"
#include "Engine/World.h"
#include "Templates/SharedPointer.h"

namespace
{
//148114910: C++ static initialization guard gives original thread-safe lifetime.
// No dtor registration is needed for the original POD/weak-index state.
FControllerMask225b GControllerMask;

class FControllerTickable225b final : public FTickableGameObject
{
public:
    AFortPlayerController* Controller = nullptr; // original borrowed raw+8
    uint8 UnusedByte10 = 0;
    bool bInitialized = false; // original+11 bit0
    virtual ~FControllerTickable225b() override = default; //1420f8c60 unregisters
    virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
    virtual bool IsTickable() const override { return true; }
    virtual bool IsTickableWhenPaused() const override { return false; }
    virtual bool IsTickableInEditor() const override { return false; }
    virtual UWorld* GetTickableGameObjectWorld() const override { return nullptr; }
    virtual TStatId GetStatId() const override { return TStatId(); }
    virtual void Tick(float) override //1420ff840
    {
        FControllerState225b& State = NativeControllerState141ca4780();
        if (!bInitialized)
        {
            bInitialized = true;
            State.World = Controller ? Controller->GetWorld() : nullptr;
            AFortPlayerController* Local = State.FindLocalController1420fa620();
            AFortPlayerCameraZone* Camera = Local ? Cast<AFortPlayerCameraZone>(Local->PlayerCameraManager) : nullptr;
            if (Camera)
            {
                State.Flags |= 1;
                uint8 Targets[4];
                Camera->NativeControllerTargets225b.Read(Targets);
                for (unsigned I=0; I!=4; ++I) State.Records[I].Initialize(Targets[I]);
            }
        }
        State.CheckRecords1420fe100();
    }
};
//148140fc8/fd0: actual single strong owner with separate object/control allocation,
// original non-thread-safe shared reference counters and engine unregister/free.
using FControllerTickableOwner = TSharedPtr<FControllerTickable225b, ESPMode::NotThreadSafe>;
// Trivial observation pointer; it does not construct an owner during reset.
FControllerTickableOwner* GControllerTickable = nullptr;
}

uint32 NativeReadControllerMask1493dd61a() { return GControllerMask.Read(); }
void NativeStoreControllerMask14944b355(uint32 Value) { GControllerMask.Store(Value); }
void NativeAddControllerMask142111870(AFortPlayerController*, uint16 Mask)
{
    NativeStoreControllerMask14944b355(NativeReadControllerMask1493dd61a() | uint32(Mask));
}
FControllerState225b::FControllerState225b()
{
    for (FEncodedRecord1420f8560& Record : Records) Record.Construct();
}
FControllerState225b& NativeControllerState141ca4780()
{
    static FControllerState225b State;
    return State;
}
AFortPlayerController* FControllerState225b::FindLocalController1420fa620() const
{
    UWorld* ResolvedWorld = World.Get();
    if (!ResolvedWorld) return nullptr;
    for (FConstPlayerControllerIterator It = ResolvedWorld->GetPlayerControllerIterator(); It; ++It)
    {
        AFortPlayerController* Candidate = Cast<AFortPlayerController>(It->Get());
        if (Candidate && Candidate->IsLocalController()) return Candidate;
    }
    return nullptr;
}
void FControllerState225b::CheckRecords1420fe100()
{
    AFortPlayerController* Local = FindLocalController1420fa620();
    AFortPlayerCameraZone* Camera = Local ? Cast<AFortPlayerCameraZone>(Local->PlayerCameraManager) : nullptr;
    if (!Camera || !(Flags & 1)) return;
    uint8 Targets[4];
    Camera->NativeControllerTargets225b.Read(Targets);
    const uint16 Mask = ComputeMask1420fe100(Records, Targets);
    if (Mask) NativeAddControllerMask142111870(Local, Mask);
}
void NativeStartControllerTickable1420fe440(FControllerState225b&, AFortPlayerController* Controller)
{
    if (GControllerTickable) return;
    // Register the tickable/engine registry before this owner's destructor.
    // Reverse static destruction then releases the tickable before its registry.
    static FControllerTickableOwner Owner = MakeShareable(new FControllerTickable225b());
    Owner->Controller = Controller;
    GControllerTickable = &Owner;
}
void NativeResetControllerTickable1420fa070()
{
    if (GControllerTickable && GControllerTickable->IsValid())
        (*GControllerTickable)->bInitialized = false;
}
