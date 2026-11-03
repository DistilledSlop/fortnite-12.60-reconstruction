#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonPanelActionNative.h"
#include "FCommonActionCommited.h"

// Actual GlobalInputHandler+50 array element (original stride0x90).
// This is distinct from the Panel+360 map: it has an owner in its key and
// has no dynamic/native delegate-handle tracker array.
struct FCommonLegacyGlobalAction
{
    FDataTableRowHandle Row;
    TWeakObjectPtr<UUserWidget> Owner;
    int32 InputPriority = 0;
    FCommonPanelProgressNative Progress;
    FCommonPanelCommitNative Commit;
    FCommonPanelCompleteNative Complete;
    FCommonPanelCommitNative DisabledCommit;
    FCommonInputActionHandlerData HandlerData;

    static bool Less(const FCommonLegacyGlobalAction& A, const FCommonLegacyGlobalAction& B)
    {
        // 14079e0b0: differing full rows sort by RowName, never by table address.
        if (A.Row != B.Row) return A.Row.RowName.Compare(B.Row.RowName) < 0;
        if (!A.Owner.IsValid()) return false;
        if (!B.Owner.IsValid()) return true;
        // The original compares UUserWidget+1b0, not stored InputPriority+18.
        return A.Owner.Get()->Priority > B.Owner.Get()->Priority;
    }
};
