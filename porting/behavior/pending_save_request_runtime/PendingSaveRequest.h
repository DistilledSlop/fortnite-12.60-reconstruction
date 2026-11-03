#pragma once

#include "CoreMinimal.h"

// Logical representation of the in-flight request stored in
// UFortMcpProfileWorld's request array. Extend this SAME type as the remaining
// request state is recovered; this is not a complete request constructor or an
// ABI layout. In particular it does not yet model the callback or the request's
// array of shared payloads, so do not use it to implement the full copy path.
struct FPendingSaveRequest
{
    // Original request offsets 0x48 and 0x98 respectively. Their individual
    // meanings are not yet established; writers must address these same fields.
    TSet<FString> ItemIds48;
    TSet<FString> ItemIds98;
};

// Original 0x14254c270. The argument is passed by value, so the temporary is
// destroyed on every return. Only the first in-flight request participates.
inline bool PendingSaveContainsItemId(const TArray<FPendingSaveRequest>& Requests, FString ItemId)
{
    return Requests.Num() > 0 &&
        (Requests[0].ItemIds48.Contains(ItemId) || Requests[0].ItemIds98.Contains(ItemId));
}
