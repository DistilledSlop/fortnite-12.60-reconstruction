#pragma once
#include "CoreMinimal.h"
#include "Tickable.h"
#include "EncodedRecord.h"

class UWorld;
class AFortPlayerController;
class AFortPlayerCameraZone;

// Owned once, in the same module as every recovered mask reader/writer.
// Logical representation of224's TEA pair; no cipher bytes escape the codecs.
struct FControllerMask225b
{
    uint32 Words[2] = {0, 0};
    int32 ActiveWord = 0;
    uint32 Read() const { return Words[ActiveWord]; } //1493dd61a
    void Store(uint32 Value) //14944b355 (conjugated through TEA decode)
    {
        ActiveWord = (ActiveWord + 1) % 2;
        Words[(ActiveWord + 1) % 2] = 0;
        Words[ActiveWord] = Value;
    }
};
FORTNITEGAME_API uint32 NativeReadControllerMask1493dd61a();
FORTNITEGAME_API void NativeStoreControllerMask14944b355(uint32 Value);
FORTNITEGAME_API void NativeAddControllerMask142111870(AFortPlayerController*, uint16 Mask);

// Partial native camera representation. This is owned by AFortPlayerCameraZone,
// not a side map and not a mirror of a second set of camera fields.
// Original identities: Target0=+2d91,Target1=+2da3,Target2=+2db5,Target3=+2dc7.
// Constructor141c981f0 writes4e to all four before its non-CDO setup call.
struct FControllerCameraTargets225b
{
    uint8 Target0 = 0x4e;
    uint8 Target1 = 0x4e;
    uint8 Target2 = 0x4e;
    uint8 Target3 = 0x4e;
    void Read(uint8 (&Out)[4]) const
    { Out[0]=Target0; Out[1]=Target1; Out[2]=Target2; Out[3]=Target3; }
};

struct FControllerState225b
{
    FEncodedRecord1420f8560 Records[4] = {};
    TWeakObjectPtr<UWorld> World;
    FWeakObjectPtr ReservedWeakObject; //+3c, target type not used by this chain
    uint8 Flags = 0;
    FControllerState225b();
    AFortPlayerController* FindLocalController1420fa620() const;
    void CheckRecords1420fe100();
};
FORTNITEGAME_API FControllerState225b& NativeControllerState141ca4780();
FORTNITEGAME_API void NativeStartControllerTickable1420fe440(FControllerState225b&, AFortPlayerController*);
FORTNITEGAME_API void NativeResetControllerTickable1420fa070();
