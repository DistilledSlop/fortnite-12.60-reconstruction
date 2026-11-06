#pragma once

#include "CoreMinimal.h"
#include "McpProfileBaseTypes.h"

// Overlay IDs compare case-sensitively. The ordinary FString hash may fold
// case; collisions are harmless because Matches makes the final distinction.
struct FMcpProfileOverlayKeyFuncs : TDefaultMapHashableKeyFuncs<FString, FMcpProfileItemPtr, false>
{
    static bool Matches(const FString& A, const FString& B)
    {
        return A.Equals(B, ESearchCase::CaseSensitive);
    }
};
using FMcpProfileOverlayMap = TMap<FString, FMcpProfileItemPtr, FDefaultSetAllocator, FMcpProfileOverlayKeyFuncs>;

// Partial logical state belonging to the same derived backend that owns the
// base groups. This is not a complete backend, binary-layout claim, or cache
// to be kept in parallel with another owner.
struct FMcpProfileOverlayState
{
    FMcpProfileOverlayMap OverridesByItemId;
    FMcpProfileGroupMap CachedGroups;
    TSet<FString> DirtyGroups;
};
