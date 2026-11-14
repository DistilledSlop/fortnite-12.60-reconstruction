// Generated from profile_entry_lifecycle_runtime; do not edit by hand.
#pragma once

#include "McpProfileBaseTypes.h"
#include "ProfileOverlayState.h"

// Partial logical declarations for the ORIGINAL backend identities, not binary
// layouts or a second index owner. Names are recovered-source names, not RTTI.
// Base identity: ctor 1410ec490 / vtable 145bfcc38.
// Derived identity: installation 14110e120 / vtable 145bfcca8.
// These declarations expose only recovered query operations and their storage.
// There is deliberately no installer, factory, profile destructor, or GC body.
// Their compiler-generated special members are NOT original lifecycle lifts.
class MCPPROFILESYS_API FMcpProfileBackendBase
{
public:
    // Original backend +0x08 and +0x58 respectively. Semantic types, no offsets.
    FMcpProfileItemMap ItemsById;
    FMcpProfileGroupMap Groups;

    // Original virtual +0x28, +0x30 and +0x38. Recompiled vtable order is not asserted.
    virtual const FMcpProfileItemArray* FindItemsByTemplateKey(const FString& Key);
    virtual void CopyItemsByTemplateKey(const FString& Key, FMcpProfileItemArray& OutItems);
    virtual FMcpProfileItemPtr FindItemById(const FString& ItemId);

protected:
    // C++ support only. Prevent unsupported allocation/deletion through this
    // partial type; future original lifecycle support must deliberately revisit.
    FMcpProfileBackendBase() = default;
    ~FMcpProfileBackendBase() = default;
};

class MCPPROFILESYS_API FMcpProfileBackendOverlay : public FMcpProfileBackendBase
{
public:
    // ORIGINAL derived-only +0x198 item projection, +0x298 cache, +0x2e8 dirty
    // set, using the existing partial type. Larger native overlay records have
    // unrecovered fields. No separate canonical owner may coexist with this.
    FMcpProfileOverlayState Overlay;

    const FMcpProfileItemArray* FindItemsByTemplateKey(const FString& Key) override;
    void CopyItemsByTemplateKey(const FString& Key, FMcpProfileItemArray& OutItems) override;
    FMcpProfileItemPtr FindItemById(const FString& ItemId) override;

protected:
    FMcpProfileBackendOverlay() = default;
    ~FMcpProfileBackendOverlay() = default;
};
