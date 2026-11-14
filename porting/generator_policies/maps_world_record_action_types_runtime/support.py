"""Owned logical world-record action layout; offsets name original fields, not host ABI."""
import hashlib
from pathlib import Path

IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
HEADER = 'FortWorldRecordAction.h'
TYPE = 'FFortWorldRecordAction'
FIELD = 'NativeWorldRecordActions_0x6c0'


def sources():
    windows = (
        (0x141f7d310, 296, 'fcab2f4f4e4c813dbb882ae5e41868510364ba250f41588cd404d3fdff25b22c'),
        (0x141f7e890, 299, '1a8109ad3fe111687e28e96f667c5d01543d9015071983f272cba1e8447007ec'),
        (0x141f7e7e0, 164, '3a949e204b0e25898ca0b1d1452d5a7ce3d146e7e0c05cfbf6ee93eb28011ceb'),
        # Pump passes false on failure, true on success, to the copied delegate.
        (0x141f93dd8, 19, '28506496b3bef896464ac5030d62bc2df54b66ed3bb87d12e359b3295321d1ae'),
        (0x141f93f9d, 20, '6f930c8327537e95b26cf3d5cd9375c85483b5820da17f3ab63971decabff6c2'),
        # The kickoff's bound callback is a direct jump to completion(bool).
        (0x141fa19a0, 5, '3fc2f251d17e7a86af6c85153cc38b954080ba57bcaace9311b00857e1aca803'),
    )
    with IMAGE.open('rb') as stream:
        for address, length, digest in windows:
            stream.seek(address - 0x140000000)
            if hashlib.sha256(stream.read(length)).hexdigest() != digest:
                raise ValueError('world record action original capture changed: ' + hex(address))
    return {HEADER: r'''// Generated from maps_world_record_action_types_runtime/support.py.
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
'''}


def validate(ctx, snippets_by_class):
    """Authorize this support header only for its exact owning member field."""
    users = [e for entries in snippets_by_class.values() for e in entries
             if HEADER in (e.get('includes') or []) or TYPE in e.get('signature', '')]
    if not users:
        return
    for name in ('AFortWorldManager', 'UFortZoneTheme', 'UFortMissionGenerator'):
        if ctx['class_pkg'].get(name) != '/Script/FortniteGame':
            raise ValueError('world record action class dependency changed: ' + name)
    for name, size in (('FFortZoneInstanceDetails', 0x128), ('FFortWindIntensityAndDirection', 8)):
        info = ctx['struct_index'].get(name)
        if (info is None or info.kind != 'struct' or info.package != '/Script/FortniteGame'
                or info.header_name != name + '.h' or info.rec.get('size') != size):
            raise ValueError('world record action struct dependency changed: ' + name)
    if len(users) != 1:
        raise ValueError('world record action support must have one owning field')
    entry = users[0]
    if (entry.get('class') != 'AFortWorldManager' or entry.get('kind') != 'member_field'
            or entry.get('function') != FIELD or entry.get('access') != 'public'
            or entry.get('signature') != 'TArray<FFortWorldRecordAction> ' + FIELD
            or int(str(entry.get('offset')), 0) != 0x6c0
            or entry.get('includes') != [HEADER]):
        raise ValueError('world record action owning field contract changed')
