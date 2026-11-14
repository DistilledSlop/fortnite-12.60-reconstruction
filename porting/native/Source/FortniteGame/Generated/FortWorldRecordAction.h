// Generated from maps_world_record_action_types_runtime/support.py.
#pragma once
#include "CoreMinimal.h"
#include "FFortZoneInstanceDetails.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Delegates/Delegate.h"
#include "HAL/PlatformTime.h"

DECLARE_DELEGATE_OneParam(FFortWorldRecordActionCompletion, bool);

// Original stride 0x38. Host layout follows the actual UE types.
struct FFortWorldRecordActionEntry
{
    FUniqueNetIdRepl UniqueId_0x0;
    bool Flag_0x28;
    TWeakObjectPtr<UObject> Object_0x2c;
};

// Original stride 0x1f8. Every owned member retains its normal UE lifetime.
// No byte-offset access or binary serialization of this host layout is valid.
struct FFortWorldRecordAction
{
    uint8 Action_0x0;
    uint8 PreviousState_0x1;
    uint8 State_0x2;
    FString String_0x8;
    FFortZoneInstanceDetails ZoneInstanceInfo;
    FString String_0x140;
    int32 Value_0x150;
    FString String_0x158;
    FString String_0x168;
    FFortWorldRecordActionCompletion Delegate_0x178;
    double StartTime_0x188;
    double CreationTime_0x190;
    FUniqueNetIdRepl UniqueId_0x198;
    TArray<FFortWorldRecordActionEntry> Entries_0x1c0;
    // Cloud-file counter IDs: exact original typedef/signedness unknown.
    TArray<uint64> RequestIds_0x1d0;
    TArray<FUniqueNetIdRepl> UniqueIds_0x1e0;
    bool Flag_0x1f0; // Constructor does not initialize this state byte.
    bool Flag_0x1f1;

    FFortWorldRecordAction(uint8 Action, const FFortZoneInstanceDetails& Details, int32 Value)
        : Action_0x0(Action), PreviousState_0x1(11), State_0x2(0),
          ZoneInstanceInfo(Details), Value_0x150(Value), StartTime_0x188(0.0),
          CreationTime_0x190(OriginalSeconds()), Flag_0x1f1(true)
    {
    }

private:
    static double OriginalSeconds()
    {
        // Windows Seconds already adds the original precision-loss guard.
        // Unix Seconds in the target UE tree returns raw monotonic seconds.
#if PLATFORM_WINDOWS
        return FPlatformTime::Seconds();
#else
        return FPlatformTime::Seconds() + 16777216.0;
#endif
    }
};
