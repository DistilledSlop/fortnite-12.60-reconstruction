#pragma once

// Data/storage and creation entry only. Routing and lifecycle registration remain separate.
#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"
#include "FUIInputAction.h"
#include "FCommonInputActionData.h"

DECLARE_DELEGATE_OneParam(FCommonActionBindingProgress, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonActionBindingProgressMulticast, float);

COMMONUI_API DECLARE_LOG_CATEGORY_EXTERN(LogUIActionRouter, Log, All);

class UWidget;
class FCommonActionBindingRoute; // Recovered route hierarchy is in CommonActionBindingRoute.h.

struct FCommonActionBindingDescriptor
{
    FName Tag;
    FDataTableRowHandle Row;
    uint8 InputMode;
    int32 Event;
    uint8 Flag20;
    uint8 Flag21;
    uint8 Flag22;
    FText DisplayOverride;
    FSimpleDelegate Click;
    FCommonActionBindingProgress Progress;
};

struct FCommonActionBindingKeyTime
{
    FKey Key;
    float Time;
};

struct FCommonActionBindingRecord
{
    FName Name;
    int32 Event;
    uint8 Flag21;
    uint8 Flag20;
    TWeakObjectPtr<UObject> Owner;
    uint8 InputMode;
    uint8 Flag22;
    FText Display;
    TWeakPtr<FCommonActionBindingRoute> Route;
    FSimpleDelegate Click;
    int32 Id;
    TArray<FCommonActionBindingKeyTime> Press;
    TArray<FCommonActionBindingKeyTime> Hold;
    FCommonActionBindingProgressMulticast ProgressListeners;
    FDataTableRowHandle Row;
    double State;
};

// The adapter must perform the original settings/row and owner-to-gamepad lookups.
// References must remain valid for the duration of BuildRecord. Missing data is a
// precondition failure; the source constructor dereferences it rather than falling back.
class ICommonActionBindingDataResolver
{
public:
    virtual ~ICommonActionBindingDataResolver() = default;
    virtual const FUIInputAction& ResolveTag(FName Tag) = 0;
    virtual const FCommonInputActionData& ResolveRow(const FDataTableRowHandle& Row) = 0;
    virtual int32 ResolveGamepadIndex(UObject* Owner) = 0;
};

namespace CommonActionBindingData
{
    // Original creation only: validates and inserts. Router association is separate.
    COMMONUI_API int32 Create(const FCommonActionBindingDescriptor& Descriptor, UWidget* Owner);
    // Isolated modulo counter step; does not reset or expose the global counter.
    COMMONUI_API int32 AllocateIdBits(uint32& Counter);
    COMMONUI_API FCommonActionBindingRecord BuildRecord(const FCommonActionBindingDescriptor& Descriptor, UObject* Owner, ICommonActionBindingDataResolver& Resolver);
    COMMONUI_API FCommonActionBindingRecord* Find(int32 Key);
    COMMONUI_API FCommonActionBindingRecord& Add(int32 Key, const FCommonActionBindingRecord& Record);
    COMMONUI_API int32 Remove(int32 Key);
    // A structural map mutation can invalidate returned pointers or references.
// Add inside the existing namespace in CommonActionBindingData.h.
COMMONUI_API void CompactInvalidBindings(); // 1407ae040

}
