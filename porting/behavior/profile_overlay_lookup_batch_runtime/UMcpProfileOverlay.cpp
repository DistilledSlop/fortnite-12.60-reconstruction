// Integration bodies for UMcpProfile. The declarations are in the companion
// file and must be added to UMcpProfile before compiling these definitions.
#include "UMcpProfile.h"
#include "ProfileOverlayState.h"
#include "McpProfileItem.h"

int32 UMcpProfile::NativeRecomputeOverlayProfileGroup(const FMcpProfileGroupMap& BaseGroups, const FMcpProfileOverlayMap& Overrides, const FString& Key, FMcpProfileItemArray& OutItems)
{
    OutItems.Empty();
    const FMcpProfileItemArray* Base = NativeFindBaseProfileGroup(BaseGroups, Key);
    if (!Base)
    {
        return 0;
    }

    int32 Matches = 0;
    for (const FMcpProfileItemPtr& Item : *Base)
    {
        // Both source pointers are required valid, as in the recovered path.
        const FMcpProfileItemPtr* Replacement = Overrides.Find(Item->ItemId);
        if (!Replacement)
        {
            OutItems.Add(Item);
        }
        else
        {
            ++Matches;
            if ((*Replacement)->Quantity > 0)
            {
                OutItems.Add(*Replacement);
            }
        }
    }
    return Matches;
}

const FMcpProfileItemArray* UMcpProfile::NativeFindOverlayProfileGroup(const FMcpProfileGroupMap& BaseGroups, FMcpProfileOverlayState& Overlay, const FString& Key)
{
    if (Overlay.DirtyGroups.Remove(Key) == 0)
    {
        if (const FMcpProfileItemArray* Cached = Overlay.CachedGroups.Find(Key))
        {
            return Cached;
        }
        return NativeFindBaseProfileGroup(BaseGroups, Key);
    }

    FMcpProfileItemArray Combined;
    if (NativeRecomputeOverlayProfileGroup(BaseGroups, Overlay.OverridesByItemId, Key, Combined) == 0)
    {
        Overlay.CachedGroups.Remove(Key);
        return NativeFindBaseProfileGroup(BaseGroups, Key);
    }

    FMcpProfileItemArray& Cached = Overlay.CachedGroups.FindOrAdd(Key);
    Cached = Combined; // independent owning shared references, including empty results
    return &Cached;
}
