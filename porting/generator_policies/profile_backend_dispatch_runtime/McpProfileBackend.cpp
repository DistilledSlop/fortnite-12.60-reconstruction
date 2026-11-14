#include "McpProfileBackend.h"
#include "UMcpProfile.h"

// Semantic member adapters for canonical bodies already attributed to these
// addresses. Do NOT add four more recovered-body counts for these adapters.
const FMcpProfileItemArray* FMcpProfileBackendBase::FindItemsByTemplateKey(const FString& Key)
{
    return UMcpProfile::NativeFindProfileGroup(Groups, Key); // 1410fe1c0
}

const FMcpProfileItemArray* FMcpProfileBackendOverlay::FindItemsByTemplateKey(const FString& Key)
{
    return UMcpProfile::NativeFindOverlayProfileGroup(Groups, Overlay, Key); // 1410fe220
}

FMcpProfileItemPtr FMcpProfileBackendBase::FindItemById(const FString& ItemId)
{
    return UMcpProfile::NativeFindBaseProfileItem(ItemsById, ItemId); // 1410fdd30
}

FMcpProfileItemPtr FMcpProfileBackendOverlay::FindItemById(const FString& ItemId)
{
    return UMcpProfile::NativeFindOverlayProfileItem(ItemsById, Overlay, ItemId); // 1410fde50
}
